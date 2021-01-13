// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fs/mount_channel.h>
#include <fs/vfs.h>

namespace fs {

MountChannel::~MountChannel() {
  if (client_end_.is_valid()) {
    // Note: this is best-effort, and would fail if the remote endpoint
    // does not speak the |fuchsia.io/DirectoryAdmin| protocol.
    fidl::ClientEnd<llcpp::fuchsia::io::DirectoryAdmin> admin(client_end_.TakeChannel());
    Vfs::UnmountHandle(std::move(admin), zx::time::infinite());
  }
}

}  // namespace fs
