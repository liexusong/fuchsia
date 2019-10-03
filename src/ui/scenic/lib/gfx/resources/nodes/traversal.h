// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_SCENIC_LIB_GFX_RESOURCES_NODES_TRAVERSAL_H_
#define SRC_UI_SCENIC_LIB_GFX_RESOURCES_NODES_TRAVERSAL_H_

#include "src/ui/scenic/lib/gfx/resources/import.h"
#include "src/ui/scenic/lib/gfx/resources/nodes/node.h"

namespace scenic_impl {
namespace gfx {

//
// Front-to-back traversals.
// Applies the functor to direct descendants in front-to-back order.
//
// This is the order in which they should be drawn to ensure that objects
// at the same elevation correctly obscure one another.
//
// The functor's signature must be |void(Node* node)|.
//
template <typename Callable>
void ForEachChildFrontToBack(const Node& node, const Callable& func) {
  // Process most recently added children first.
  for (auto it = node.children().rbegin(); it != node.children().rend(); ++it) {
    func(it->get());
  }
}

template <typename Callable>
void ForEachImportFrontToBack(const Node& node, const Callable& func) {
  // Process most recently added imports first.
  for (auto it = node.imports().rbegin(); it != node.imports().rend(); ++it) {
    func(static_cast<Node*>((*it)->delegate()));
  }
}

template <typename Callable>
void ForEachChildAndImportFrontToBack(const Node& node, const Callable& func) {
  ForEachChildFrontToBack(node, func);
  ForEachImportFrontToBack(node, func);
}

template <typename Callable>
void ForEachDirectDescendantFrontToBack(const Node& node, const Callable& func) {
  ForEachChildAndImportFrontToBack(node, func);
}

//
// Traversals with early termination once the functor returns true.
//
// The functor's signature must be |bool(const Node* node)|.
//
template <typename Callable>
bool ForEachChildFrontToBackUntilTrue(const Node& node, const Callable& func) {
  // Process most recently added children first.
  for (auto it = node.children().rbegin(); it != node.children().rend(); ++it) {
    if (func(it->get()))
      return true;
  }
  return false;
}

template <typename Callable>
bool ForEachImportFrontToBackUntilTrue(const Node& node, const Callable& func) {
  // Process most recently added imports first.
  for (auto it = node.imports().rbegin(); it != node.imports().rend(); ++it) {
    if (func(static_cast<Node*>((*it)->delegate())))
      return true;
  }
  return false;
}

template <typename Callable>
bool ForEachChildAndImportFrontToBackUntilTrue(const Node& node, const Callable& func) {
  return ForEachChildFrontToBackUntilTrue(node, func) ||
         ForEachImportFrontToBackUntilTrue(node, func);
}

}  // namespace gfx
}  // namespace scenic_impl

#endif  // SRC_UI_SCENIC_LIB_GFX_RESOURCES_NODES_TRAVERSAL_H_
