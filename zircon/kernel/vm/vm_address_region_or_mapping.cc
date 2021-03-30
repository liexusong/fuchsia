// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <trace.h>
#include <zircon/errors.h>
#include <zircon/types.h>

#include <vm/vm.h>
#include <vm/vm_aspace.h>

#include "vm/vm_address_region.h"
#include "vm_priv.h"

#define LOCAL_TRACE VM_GLOBAL_TRACE(0)

VmAddressRegionOrMapping::VmAddressRegionOrMapping(vaddr_t base, size_t size, uint32_t flags,
                                                   VmAspace* aspace, VmAddressRegion* parent,
                                                   bool is_mapping)
    : is_mapping_(is_mapping),
      state_(LifeCycleState::NOT_READY),
      base_(base),
      size_(size),
      flags_(flags),
      aspace_(aspace),
      parent_(parent) {
  LTRACEF("%p\n", this);
}

zx_status_t VmAddressRegionOrMapping::Destroy() {
  canary_.Assert();

  Guard<Mutex> guard{aspace_->lock()};
  if (state_ != LifeCycleState::ALIVE) {
    return ZX_ERR_BAD_STATE;
  }

  return DestroyLocked();
}

VmAddressRegionOrMapping::~VmAddressRegionOrMapping() {
  LTRACEF("%p\n", this);

  if (state_ == LifeCycleState::ALIVE) {
    Destroy();
  }

  DEBUG_ASSERT(!this->in_subregion_tree());
}

bool VmAddressRegionOrMapping::IsAliveLocked() const {
  canary_.Assert();
  DEBUG_ASSERT(aspace_->lock()->lock().IsHeld());
  return state_ == LifeCycleState::ALIVE;
}

fbl::RefPtr<VmAddressRegion> VmAddressRegionOrMapping::as_vm_address_region() {
  canary_.Assert();
  if (is_mapping()) {
    return nullptr;
  }
  return fbl::RefPtr<VmAddressRegion>(static_cast<VmAddressRegion*>(this));
}

fbl::RefPtr<VmMapping> VmAddressRegionOrMapping::as_vm_mapping() {
  canary_.Assert();
  if (!is_mapping()) {
    return nullptr;
  }
  return fbl::RefPtr<VmMapping>(static_cast<VmMapping*>(this));
}

VmAddressRegion* VmAddressRegionOrMapping::as_vm_address_region_ptr() {
  canary_.Assert();
  if (unlikely(is_mapping())) {
    return nullptr;
  }
  return static_cast<VmAddressRegion*>(this);
}

VmMapping* VmAddressRegionOrMapping::as_vm_mapping_ptr() {
  canary_.Assert();
  if (unlikely(!is_mapping())) {
    return nullptr;
  }
  return static_cast<VmMapping*>(this);
}

bool VmAddressRegionOrMapping::is_valid_mapping_flags(uint arch_mmu_flags) {
  if (!(flags_ & VMAR_FLAG_CAN_MAP_READ) && (arch_mmu_flags & ARCH_MMU_FLAG_PERM_READ)) {
    return false;
  }
  if (!(flags_ & VMAR_FLAG_CAN_MAP_WRITE) && (arch_mmu_flags & ARCH_MMU_FLAG_PERM_WRITE)) {
    return false;
  }
  if (!(flags_ & VMAR_FLAG_CAN_MAP_EXECUTE) && (arch_mmu_flags & ARCH_MMU_FLAG_PERM_EXECUTE)) {
    return false;
  }
  return true;
}

size_t VmAddressRegionOrMapping::AllocatedPages() const {
  Guard<Mutex> guard{aspace_->lock()};
  if (state_ != LifeCycleState::ALIVE) {
    return 0;
  }
  return AllocatedPagesLocked();
}
