#!/usr/bin/env python3.8
# Copyright 2020 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Use scrutiny to verify a build."""

import argparse
import difflib
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile

SUPPORTED_TYPES = ['kernel_cmdline', 'bootfs_filelist', 'static_pkgs']

SOFT_TRANSITION_MESSAGE_TEMPLATE = """
If you are making a change in fuchsia repo that causes this you need a soft transition by:
1: copy the old golden file to *.orig.
2: update the original golden file to a new golden file as suggested above.
3: modify the product configuration GNI file where `{0}` or `{1}` is defined to contain both the old golden file and the new golden file.
4: check in your fuchsia change.
5: remove the original golden file and remove the entry from `{0}` or `{1}`.
"""


def print_error(msg):
    print(msg, file=sys.stderr)


def main(input_args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--zbi-file', help='Path to the zbi to verify', required=True)
    parser.add_argument(
        '--blobfs-manifest',
        help='Path to blobfs manifest file, required for "static_pkgs"',
        required=False)
    parser.add_argument(
        '--scrutiny',
        help='Path to the scrutiny tool used for verifying kernel cmdline',
        required=True)
    parser.add_argument(
        '--far',
        help=(
            'Path to the far tool used for extracting package, ' +
            'required for "static_pkgs"'),
        required=False)
    parser.add_argument(
        '--golden-files',
        help=(
            'Path to one of the possible golden files to check against, ' +
            'there should only be one golden file in normal case, and only ' +
            'two golden files, one old file and one new file during a soft ' +
            'transition. After the transition, the old golden file should ' +
            'be removed and only leave the new golden file.'),
        nargs='+',
        required=True)
    parser.add_argument(
        '--stamp', help='Path to the victory file', required=True)
    parser.add_argument(
        '--type',
        help=('The type of the ZBI item to verify'),
        choices=SUPPORTED_TYPES,
        required=True)
    parser.add_argument(
        '--depfile',
        help=(
            'Optional generated depfile listing dynamic deps for the script' +
            ', required for "static_pkgs"'),
        required=False)
    args = parser.parse_args(input_args)

    if len(args.golden_files) > 2:
        print_error(
            'At most two optional golden files are supported, ' +
            'is there a soft transition already in place? Please wait for ' +
            'that to finish before starting a new one.')
    try:
        verify_build(args)
    except VerificationError as e:
        print_error(str(e))
        return 1

    with open(args.stamp, 'w') as stamp_file:
        stamp_file.write('Golden!\n')
    return 0


def verify_build(args):
    """verify_build verifies a build against specified golden files.

    Raises:
        VerificationError: If verification fails.
    """
    # Check for some necessary files/dirs exist first.
    for file in [args.scrutiny, args.zbi_file]:
        if not os.path.exists(file):
            raise VerificationError('Missing required file: ' + file)

    with tempfile.TemporaryDirectory() as tmp:
        run_scrutiny_command(
            args.scrutiny, ' '.join(
                [
                    'tool.zbi.extract', '--input',
                    shlex.quote(args.zbi_file), '--output',
                    shlex.quote(tmp)
                ]))

        last_error = None
        for golden_file in args.golden_files:
            try:
                if args.type == 'kernel_cmdline':
                    verify_kernel_cmdline(golden_file, tmp)
                elif args.type == 'bootfs_filelist':
                    verify_bootfs_filelist(golden_file, tmp)
                elif args.type == 'static_pkgs':
                    verify_static_pkgs(args, golden_file, tmp)
                # Passes the verification, no error thrown.
                return
            except VerificationError as e:
                # Error thrown, we want to record this error and check next
                # golden_file.
                last_error = e

        raise last_error


def verify_kernel_cmdline(kernel_cmdline_golden_file, scrutiny_out):
    """verify_kernel_cmdline verifies the kernel cmdline in ZBI image.

    Raises:
        VerificationError: If verification fails.
    """
    try:
        with open(kernel_cmdline_golden_file, 'r') as f:
            golden_file_content = f.read().strip()
    except IOError as e:
        raise VerificationError(f'Failed to open golden file: {e}')
    if not os.path.exists(os.path.join(scrutiny_out, 'sections',
                                       'cmdline.blk')):
        # We find no kernel cmdline. Check whether the golden file is empty.
        if not golden_file_content:
            # Golden file is empty. Pass the check.
            return
        else:
            error_msg = (
                'Found no kernel cmdline in ZBI\n' +
                'Please update kernel cmdline golden file at ' +
                kernel_cmdline_golden_file + ' to be an empty file')
            raise VerificationError(error_msg)
    try:
        with open(os.path.join(scrutiny_out, 'sections', 'cmdline.blk'),
                  'r') as f:
            # The cmdline.blk contains a trailing \x00.
            cmdline = f.read().strip().rstrip('\x00')
    except IOError as e:
        raise VerificationError(f'Failed to read cmdline.blk: {e}')

    try:
        compare_cmdline(
            cmdline, golden_file_content, kernel_cmdline_golden_file)
    except CmdlineFormatError as e:
        raise VerificationError(f'Invalid cmdline format: {e}')
    return


def verify_bootfs_filelist(bootfs_filelist_golden_file, scrutiny_out):
    """verify_bootfs_filelist verifies the bootFS filelist in ZBI image.

    Raises:
      VerificationError: If verification fails.
    """
    try:
        with open(bootfs_filelist_golden_file, 'r') as f:
            golden_file_content = f.read().strip()
    except IOError as e:
        raise VerificationError(f'Failed to read golden file: {e}')
    bootfs_folder = os.path.join(scrutiny_out, 'bootfs')
    bootfs_files = []
    try:
        for root, _, files in os.walk(bootfs_folder):
            for file in files:
                bootfs_files.append(
                    os.path.relpath(os.path.join(root, file), bootfs_folder))
    except IOError as e:
        raise VerificationError(f'Failed to walk bootfs folder: {e}')
    got_content = '\n'.join(sorted(bootfs_files))

    if golden_file_content == got_content:
        return
    error_msgs = ['BootFS file list mismatch!']
    error_msgs.append(
        'Please update bootFS file list golden file at ' +
        bootfs_filelist_golden_file + ' to:')
    error_msgs.append('```')
    error_msgs.append(got_content)
    error_msgs.append('```')
    error_msgs.append('')
    error_msgs.append('Diff:')
    error_msgs.extend(
        difflib.context_diff(
            golden_file_content.splitlines(keepends=True),
            got_content.splitlines(keepends=True),
            fromfile='want',
            tofile='got'))
    error_msgs.append(
        SOFT_TRANSITION_MESSAGE_TEMPLATE.format(
            'fuchsia_zbi_bootfs_filelist_goldens',
            'recovery_zbi_bootfs_filelist_goldens'))
    raise VerificationError('\n'.join(error_msgs))


def verify_static_pkgs(
    args,
    golden_file,
    scrutiny_out,
):
    """verify_static_pkgs verifies static packages list.

    Raises:
      VerificationError: If verification fails.
    """
    deps = []
    if not args.blobfs_manifest:
        raise VerificationError(
            '"blobfs-manifest" must be specified for "static_pkgs" check')
    if not args.far:
        raise VerificationError(
            '"far" must be specified for "static_pkgs" check')
    if not args.depfile:
        raise VerificationError(
            '"depfile" must be specified for "static_pkgs" check')
    try:
        system_image_hash = get_system_image_hash(scrutiny_out)
    except IOError as e:
        raise VerificationError(f'Failed to get devmgr config: {e}')
    except KeyError as e:
        raise VerificationError(f'Invalid devmgr config: {e}')

    try:
        blob_manifest = parse_key_value_file(args.blobfs_manifest)
    except IOError as e:
        raise VerificationError(f'Failed to open blob manifest: {e}')

    try:
        system_image_blob = os.path.join(
            os.path.dirname(args.blobfs_manifest),
            blob_manifest[system_image_hash])
        # Add system_image_blob as dynamic dependency.
        deps.append(system_image_blob)
    except KeyError as e:
        raise VerificationError(f'System image blob not found: {e}')
    system_image_folder = os.path.join(scrutiny_out, 'system_image')
    try:
        extract_package(args.far, system_image_blob, system_image_folder)
    except subprocess.CalledProcessError as e:
        raise VerificationError(
            f'Failed to extract system_image package: {e.stderr}')

    try:
        static_packages_hash = parse_key_value_file(
            os.path.join(system_image_folder, 'meta',
                         'contents'))['data/static_packages']
    except KeyError:
        raise VerificationError(
            'No "data/static_packages" found in "system_image"')
    except IOError as e:
        raise VerificationError(
            f'Failed to read system_image/meta/contents file: {e}')
    try:
        static_packages_blob = os.path.join(
            os.path.dirname(args.blobfs_manifest),
            blob_manifest[static_packages_hash])

        # Add static_packages_blob as dynamic dependency.
        deps.append(static_packages_blob)
    except KeyError as e:
        raise VerificationError(f'Static pkgs blob not found: {e}')
    try:
        with open(static_packages_blob, 'r') as f:
            static_packages_content = f.read().strip()
    except IOError as e:
        raise VerificationError(f'Failed to read static packages blob: {e}')

    # Write depfile.
    try:
        with open(args.depfile, 'w') as f:
            f.write(args.stamp + ': ' + ' '.join(deps) + '\n')
    except IOError as e:
        raise VerificationError(f'Failed to write depfile: {e}')

    pkgs = []
    for pkg in static_packages_content.splitlines():
        pkgs.append(re.split(r'/[0-9]=', pkg)[0])
    got_content = '\n'.join(sorted(pkgs))

    try:
        with open(golden_file, 'r') as f:
            golden_file_content = f.read().strip()
    except IOError as e:
        raise VerificationError(f'Failed to read golden file: {e}')

    if golden_file_content == got_content:
        return
    error_msgs = ['Static packages list mismatch!']
    error_msgs.append(
        'Please update static packages list golden file at ' + golden_file +
        ' to:')
    error_msgs.append('```')
    error_msgs.append(got_content)
    error_msgs.append('```')
    error_msgs.append('')
    error_msgs.append('Diff:')
    error_msgs.extend(
        difflib.context_diff(
            golden_file_content.splitlines(keepends=True),
            got_content.splitlines(keepends=True),
            fromfile='want',
            tofile='got'))
    error_msgs.append(
        SOFT_TRANSITION_MESSAGE_TEMPLATE.format(
            'fuchsia_static_pkgs_goldens', 'recovery_static_pkgs_goldens'))
    raise VerificationError('\n'.join(error_msgs))


def get_system_image_hash(scrutiny_out):
    """Get the system image merkle root.

    Args:
        scrutiny_out: the scrutiny output directory.

    Raises:
        IOError: If fails to read devmgr config.
        KeyError: If the config entry for system image hash is not found.
    """
    devmgr_config_file = os.path.join(
        scrutiny_out, 'bootfs', 'config', 'devmgr')
    key_value_map = parse_key_value_file(devmgr_config_file)
    return key_value_map['zircon.system.pkgfs.cmd'].replace('bin/pkgsvr+', '')


def run_scrutiny_command(scrutiny_path, command):
    """Runs scrutiny command.

    Args:
        scrutiny_path: The path to the scrutiny tool.
        command: The scrutiny command to run.

    Raises:
        VerificationError: If the command fails or the output is not
            '{"status":"ok"}'.
    """
    try:
        output = subprocess.run(
            [scrutiny_path, '-c', command], capture_output=True,
            check=True).stdout
    except subprocess.CalledProcessError as e:
        raise VerificationError(f'Failed to run scrutiny: {e.stederr}')

    try:
        if json.loads(output)['status'] != 'ok':
            raise VerificationError(f'Unexpected scrutiny output: {output}')
    except (KeyError, json.JSONDecodeError) as e:
        raise VerificationError(f'Unexpected scrutiny output: {e}')


def extract_package(far_path, package_path, output_dir):
    """Extract a package from a blob using "fx far extract".

    Args:
        far_path: The path to far tool.
        package_path: The path to the package blob file.
        output_dir: The output directory to put the extracted package.

    Raises:
        subprocess.CalledProcessError: If failed to extract.
    """
    subprocess.run(
        [
            far_path, 'extract', '--archive=' + package_path,
            '--output=' + output_dir
        ],
        capture_output=True,
        check=True)


class CmdlineFormatError(Exception):
    """Exception thrown when kernel cmdline is in invalid format."""

    def __init__(self, msg):
        Exception.__init__(self)
        self.msg = msg

    def __str__(self):
        return self.msg


class VerificationError(Exception):
    """Exception thrown when verification fails."""

    def __init__(self, msg):
        Exception.__init__(self)
        self.msg = msg

    def __str__(self):
        return self.msg


def compare_cmdline(actual_cmdline, golden_cmdline, golden_file):
    """compare_cmdline compares the actual cmdline with the golden cmdline.

    Raises:
      CmdlineFormatError: If the kernel cmdline is not formatted correctly.
    """
    golden_cmd = generate_sorted_cmdline(golden_cmdline, '\n')
    actual_cmd = generate_sorted_cmdline(actual_cmdline, ' ')
    if golden_cmd == actual_cmd:
        return
    error_msgs = ['Kernel cmdline mismatch!']
    error_msgs.append(
        'Please update kernel cmdline golden file at ' + golden_file + ' to:')
    error_msgs.append('```')
    error_msgs.append(actual_cmd)
    error_msgs.append('```')
    error_msgs.append('')
    error_msgs.append('Diff:')
    error_msgs.extend(
        difflib.context_diff(
            golden_cmd.splitlines(keepends=True),
            actual_cmd.splitlines(keepends=True),
            fromfile='want',
            tofile='got'))
    error_msgs.append(
        SOFT_TRANSITION_MESSAGE_TEMPLATE.format(
            'fuchsia_zbi_kernel_cmdline_goldens',
            'recovery_zbi_kernel_cmdline_goldens'))
    raise VerificationError('\n'.join(error_msgs))


def generate_sorted_cmdline(cmdline, splitter):
    """generate_sorted_cmdline generates a kernel cmdline sorted by entry keys.

    Raises:
      CmdlineFormatError: If the kernel cmdline is not formatted correctly.
    """
    cmdline_entries = {}
    entries = cmdline.split(splitter)
    for entry in entries:
        if len(entry.split('=')) > 2:
            raise CmdlineFormatError(
                'invalid kernel cmdline, key value pair: ' + entry)
        key, _, value = entry.partition('=')
        if key in cmdline_entries:
            raise CmdlineFormatError('duplicate kernel cmdline key: ' + key)
        cmdline_entries[key] = value

    return '\n'.join(
        ('%s=%s' % (key, value)) if value else key
        for key, value in sorted(cmdline_entries.items()))


def parse_key_value_file(file_path):
    """Parses a file in 'key=value' format.

  Args:
    file_path: The path to the file.

  Returns:
    A {key:value} map

  Raises:
    IOError: if failed to read the file.
  """
    with open(file_path, 'r') as f:
        content = f.read()
    key_value_map = {}
    for line in content.splitlines():
        split_array = line.split('=')
        if len(split_array) == 2:
            key_value_map[split_array[0]] = split_array[1]
    return key_value_map


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
