// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_SCENIC_LIB_FLATLAND_ALLOCATOR_H_
#define SRC_UI_SCENIC_LIB_FLATLAND_ALLOCATOR_H_

#include <fuchsia/sysmem/cpp/fidl.h>
#include <fuchsia/ui/scenic/internal/cpp/fidl.h>

#include <unordered_set>

#include "src/lib/fxl/memory/weak_ptr.h"
#include "src/ui/scenic/lib/flatland/buffers/buffer_collection_importer.h"
#include "src/ui/scenic/lib/sysmem/id.h"

namespace flatland {

// This class implements Allocator service which allows allocation of BufferCollections which can be
// used in multiple Flatland/Gfx sessions simultaneously.
class Allocator : public fuchsia::ui::scenic::internal::Allocator {
 public:
  Allocator(
      const std::vector<std::shared_ptr<BufferCollectionImporter>>& buffer_collection_importers,
      fuchsia::sysmem::AllocatorSyncPtr sysmem_allocator);
  ~Allocator() override;

  // |fuchsia::ui::scenic::internal::Allocator|
  void RegisterBufferCollection(
      fuchsia::ui::scenic::internal::BufferCollectionExportToken export_token,
      fidl::InterfaceHandle<fuchsia::sysmem::BufferCollectionToken> buffer_collection_token,
      RegisterBufferCollectionCallback callback) override;

  const std::vector<std::shared_ptr<BufferCollectionImporter>>& buffer_collection_importers()
      const {
    return buffer_collection_importers_;
  }

 private:
  void ReleaseBufferCollection(sysmem_util::GlobalBufferCollectionId collection_id);

  // Used to import Flatland buffer collections and images to external services that Flatland does
  // not have knowledge of. Each importer is used for a different service.
  std::vector<std::shared_ptr<BufferCollectionImporter>> buffer_collection_importers_;

  // A Sysmem allocator to faciliate buffer allocation with the Renderer.
  fuchsia::sysmem::AllocatorSyncPtr sysmem_allocator_;

  // Keep track of buffer collection Ids for garbage collection.
  std::unordered_set<sysmem_util::GlobalBufferCollectionId> buffer_collections_;

  // Should be last.
  fxl::WeakPtrFactory<Allocator> weak_factory_;
};

}  // namespace flatland

#endif  // SRC_UI_SCENIC_LIB_FLATLAND_ALLOCATOR_H_
