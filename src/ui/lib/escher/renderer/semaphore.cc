// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/lib/escher/renderer/semaphore.h"

#include "src/ui/lib/escher/impl/vulkan_utils.h"

namespace escher {

Semaphore::Semaphore(vk::Device device) : Semaphore(device, false) {}

Semaphore::Semaphore(vk::Device device, bool exportable) : device_(device) {
  vk::SemaphoreCreateInfo info;
#ifdef __Fuchsia__
  vk::ExternalSemaphoreHandleTypeFlags flags(
#if VK_HEADER_VERSION > 173
      vk::ExternalSemaphoreHandleTypeFlagBits::eZirconEventFUCHSIA);
#else
      vk::ExternalSemaphoreHandleTypeFlagBits::eTempZirconEventFUCHSIA);
#endif
  vk::ExportSemaphoreCreateInfoKHR export_info(flags);
  if (exportable) {
    info.pNext = &export_info;
  }
#else
  FX_DCHECK(!exportable) << "semaphore export not supported on this platform";
#endif
  value_ = ESCHER_CHECKED_VK_RESULT(device_.createSemaphore(info));
}

Semaphore::~Semaphore() { device_.destroySemaphore(value_); }

SemaphorePtr Semaphore::New(vk::Device device) { return fxl::MakeRefCounted<Semaphore>(device); }

SemaphorePtr Semaphore::NewExportableSem(vk::Device device) {
  return fxl::MakeRefCounted<Semaphore>(device, true);
}

}  // namespace escher
