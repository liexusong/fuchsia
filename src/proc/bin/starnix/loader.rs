// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use anyhow::{Context, Error};
use fidl_fuchsia_io as fio;
use fuchsia_zircon::{self as zx, AsHandleRef};
use process_builder::{elf_load, elf_parse};
use std::ffi::{CStr, CString};

use crate::mm::*;
use crate::task::*;
use crate::types::*;

fn populate_initial_stack(
    stack_vmo: &zx::Vmo,
    argv: &Vec<CString>,
    environ: &Vec<CString>,
    mut auxv: Vec<(u32, u64)>,
    stack_base: usize,
    original_stack_start_addr: usize,
) -> Result<usize, zx::Status> {
    let mut stack_pointer = original_stack_start_addr;
    let write_stack = |data: &[u8], addr: usize| stack_vmo.write(data, (addr - stack_base) as u64);

    let mut string_data = vec![];
    for arg in argv {
        string_data.extend_from_slice(arg.as_bytes_with_nul());
    }
    for env in environ {
        string_data.extend_from_slice(env.as_bytes_with_nul());
    }
    stack_pointer -= string_data.len();
    let strings_addr = stack_pointer;
    write_stack(string_data.as_slice(), strings_addr)?;

    let mut random_seed = [0; 16];
    zx::cprng_draw(&mut random_seed)?;
    stack_pointer -= random_seed.len();
    let random_seed_addr = stack_pointer;
    write_stack(&random_seed, random_seed_addr)?;

    auxv.push((AT_RANDOM, random_seed_addr as u64));
    auxv.push((AT_NULL, 0));

    // After the remainder (argc/argv/environ/auxv) is pushed, the stack pointer must be 16 byte
    // aligned. This is required by the ABI and assumed by the compiler to correctly align SSE
    // operations. But this can't be done after it's pushed, since it has to be right at the top of
    // the stack. So we collect it all, align the stack appropriately now that we know the size,
    // and push it all at once.
    let mut main_data = vec![];
    // argc
    let argc: u64 = argv.len() as u64;
    main_data.extend_from_slice(&argc.to_ne_bytes());
    // argv
    const ZERO: [u8; 8] = [0; 8];
    let mut next_string_addr = strings_addr;
    for arg in argv {
        main_data.extend_from_slice(&next_string_addr.to_ne_bytes());
        next_string_addr += arg.as_bytes_with_nul().len();
    }
    main_data.extend_from_slice(&ZERO);
    // environ
    for env in environ {
        main_data.extend_from_slice(&next_string_addr.to_ne_bytes());
        next_string_addr += env.as_bytes_with_nul().len();
    }
    main_data.extend_from_slice(&ZERO);
    // auxv
    for (tag, val) in auxv {
        main_data.extend_from_slice(&(tag as u64).to_ne_bytes());
        main_data.extend_from_slice(&(val as u64).to_ne_bytes());
    }

    // Time to push.
    stack_pointer -= main_data.len();
    stack_pointer -= stack_pointer % 16;
    write_stack(main_data.as_slice(), stack_pointer)?;

    return Ok(stack_pointer);
}

struct LoadedElf {
    headers: elf_parse::Elf64Headers,
    base: usize,
    bias: usize,
}

fn load_elf(vmo: &zx::Vmo, mm: &MemoryManager) -> Result<LoadedElf, Error> {
    let headers = elf_parse::Elf64Headers::from_vmo(&vmo).context("ELF parsing failed")?;
    let elf_info = elf_load::loaded_elf_info(&headers);
    // Allocate a vmar of the correct size, get the random location, then immediately destroy it.
    // This randomizes the load address without loading into a sub-vmar and breaking mprotect.
    // This is different from how Linux actually lays out the address space. We might need to
    // rewrite it eventually.
    let (temp_vmar, base) = mm
        .root_vmar
        .allocate(0, elf_info.high - elf_info.low, zx::VmarFlags::empty())
        .context("Couldn't allocate temporary VMAR")?;
    unsafe { temp_vmar.destroy()? }; // Not unsafe, the vmar is not in the current process
    let bias = base.wrapping_sub(elf_info.low);
    let mapper = mm as &dyn elf_load::Mapper;
    elf_load::map_elf_segments(&vmo, &headers, mapper, mm.root_vmar.info()?.base, bias)
        .context("map_elf_segments failed")?;
    Ok(LoadedElf { headers, base, bias })
}

// TODO(tbodt): change to return an errno when it's time to implement execve
// TODO(abarth): Split the loading of the executable from the starting of the thread
//               so that execve can reconfigure the memory in the process using this
//               function without needing to actually start the thread.
pub fn load_executable(
    task: &Task,
    executable: zx::Vmo,
    argv: &Vec<CString>,
    environ: &Vec<CString>,
) -> Result<(), Error> {
    // TODO: We don't keep track of the mappings created by load_elf in the MemoryManager.
    let main_elf = load_elf(&executable, &task.mm).context("Main ELF failed to load")?;
    let interp_elf = if let Some(interp_hdr) =
        main_elf.headers.program_header_with_type(elf_parse::SegmentType::Interp)?
    {
        let mut interp = vec![0; interp_hdr.filesz as usize];
        executable.read(&mut interp, interp_hdr.offset as u64)?;
        // TODO: once it exists, use the Starnix VFS to open and map the interpreter
        let mut interp = CStr::from_bytes_with_nul(&interp)?.to_str()?;
        if interp.starts_with('/') {
            interp = &interp[1..];
        }
        let interp_vmo = syncio::directory_open_vmo(
            &task.fs.root,
            interp,
            fio::VMO_FLAG_READ | fio::VMO_FLAG_EXEC,
            zx::Time::INFINITE,
        )?;
        Some(load_elf(&interp_vmo, &task.mm).context("Interpreter ELF failed to load")?)
    } else {
        None
    };

    let entry_elf = (&interp_elf).as_ref().unwrap_or(&main_elf);
    let entry = entry_elf.headers.file_header().entry.wrapping_add(entry_elf.bias);

    // TODO(tbodt): implement MAP_GROWSDOWN and then reset this to 1 page. The current value of
    // this is based on adding 0x1000 each time a segfault appears.
    let stack_size: usize = 0x5000;
    let stack_vmo = zx::Vmo::create(stack_size as u64)?;
    stack_vmo.set_name(CStr::from_bytes_with_nul(b"[stack]\0")?)?;
    let stack_base = task
        .mm
        .root_vmar
        .map(0, &stack_vmo, 0, stack_size, zx::VmarFlags::PERM_READ | zx::VmarFlags::PERM_WRITE)
        .context("failed to map stack")?;
    let stack = stack_base + stack_size - 8;

    let auxv = vec![
        (AT_UID, task.creds.uid as u64),
        (AT_EUID, task.creds.euid as u64),
        (AT_GID, task.creds.gid as u64),
        (AT_EGID, task.creds.egid as u64),
        (AT_BASE, interp_elf.map_or(0, |interp| interp.base as u64)),
        (AT_PAGESZ, *PAGE_SIZE),
        (AT_PHDR, main_elf.bias.wrapping_add(main_elf.headers.file_header().phoff) as u64),
        (AT_PHNUM, main_elf.headers.file_header().phnum as u64),
        (AT_ENTRY, main_elf.bias.wrapping_add(main_elf.headers.file_header().entry) as u64),
        (AT_SECURE, 0),
    ];
    let stack = populate_initial_stack(&stack_vmo, argv, environ, auxv, stack_base, stack)?;
    task.thread_group.process.start(&task.thread, entry, stack, zx::Handle::invalid(), 0)?;

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use fidl_fuchsia_io as fio;
    use fuchsia_async as fasync;
    use fuchsia_zircon::Task as zxTask;

    use crate::testing::*;

    #[fasync::run_singlethreaded(test)]
    async fn test_trivial_initial_stack() {
        let stack_vmo = zx::Vmo::create(0x4000).expect("VMO creation should succeed.");
        let stack_base: usize = 0x3000_0000;
        let original_stack_start_addr: usize = 0x3000_1000;

        let argv = &vec![];
        let environ = &vec![];

        let stack_start_addr = populate_initial_stack(
            &stack_vmo,
            &argv,
            &environ,
            vec![],
            stack_base,
            original_stack_start_addr,
        )
        .expect("Populate initial stack should succeed.");

        let argc_size: usize = 8;
        let argv_terminator_size: usize = 8;
        let environ_terminator_size: usize = 8;
        let aux_random: usize = 16;
        let aux_null: usize = 16;
        let random_seed: usize = 16;

        let mut payload_size = argc_size
            + argv_terminator_size
            + environ_terminator_size
            + aux_random
            + aux_null
            + random_seed;
        payload_size += payload_size % 16;

        assert_eq!(stack_start_addr, original_stack_start_addr - payload_size);
    }

    #[fasync::run_singlethreaded(test)]
    async fn test_load_hello_starnix() {
        let (_kernel, task_owner) = create_kernel_and_task();
        let task = &task_owner.task;

        let executable = syncio::directory_open_vmo(
            &task.fs.root,
            &"bin/hello_starnix",
            fio::VMO_FLAG_READ | fio::VMO_FLAG_EXEC,
            zx::Time::INFINITE,
        )
        .expect("failed to load vmo for bin/hello_starnix");

        let argv = &vec![];
        let environ = &vec![];

        // Currently, load_executable also starts the thread. We need to install
        // an exception handler so that the test framework doesn't see any
        // BAD_SYSCALL exceptions.
        let _exceptions = task_owner
            .task
            .thread
            .create_exception_channel()
            .expect("failed to create exception channel");
        load_executable(&task, executable, argv, environ).expect("failed to load executable");

        assert!(task.mm.get_mapping_count() > 0);
    }
}
