// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_SCENIC_LIB_GFX_ENGINE_HIT_TESTER_H_
#define SRC_UI_SCENIC_LIB_GFX_ENGINE_HIT_TESTER_H_

#include <vector>

#include "src/ui/scenic/lib/gfx/engine/hit.h"

namespace scenic_impl {
namespace gfx {

class Node;
class Session;

// Performs a hit test on the contents of a node.
class HitTester {
 public:
  HitTester() = default;
  virtual ~HitTester() = default;

  // Performs a hit test along the specified ray.  Returns a list of hits
  // sorted by increasing distance, then by increasing tree depth.
  //
  // The specific hit collection behavior depends on should_participate()
  // behavior, implemented by each subclass.
  std::vector<Hit> HitTest(Node* node, const escher::ray4& ray);

 protected:
  // Describes a possible hit within an enclosing tag node.
  struct TagInfo {
    static constexpr float kNoHit = std::numeric_limits<float>::infinity();

    // The distance to the intersection as defined by |Hit.distance|.
    float distance = kNoHit;

    bool is_hit() const { return distance < kNoHit; }

    void ReportIntersection(float d) {
      if (d < distance)
        distance = d;
    }
  };

  // Describes a ray and its accumulated transform.
  struct RayInfo {
    // The ray to test in the object's coordinate system.
    escher::ray4 ray;

    // The accumulated inverse transformation matrix which maps the coordinate
    // system of the node at which the hit test was initiated into the
    // coordinate system of the object.
    escher::mat4 inverse_transform;
  };

  // Used to determine hit collection behavior.
  // In a session-based hit test, nodes must have a compatible tag and session.
  // In a global hit test, all nodes participate, regardless of tag or session.
  virtual bool should_participate(Node* node) = 0;

  // Accumulates hit test results from the node, as seen by its parent.
  // Applies the node's transform to the ray stack.
  // |ray_info_| must be in the parent's local coordinate system.
  void AccumulateHitsOuter(Node* node);

  // Accumulates hit test results from the node, as seen by the node itself.
  // Applies the node's tag to the tag stack.
  // |ray_info_| must be in the node's local coordinate system.
  void AccumulateHitsLocal(Node* node);

  // Accumulates hit test results from the node's content and children.
  // |ray_info_| must be in the node's local coordinate system.
  void AccumulateHitsInner(Node* node);

  // The vector which accumulates hits.
  std::vector<Hit> hits_;

  // The current tag information.
  // Null if there is no enclosing tagged node.
  // TODO(SCN-909): Refactor out.
  TagInfo* tag_info_ = nullptr;

  // The current ray information.
  // Null if there is no hit test currently in progress.
  // TODO(SCN-909): Refactor out.
  RayInfo* ray_info_ = nullptr;

  // The current intersection information.
  // NULL if we haven't intersected anything yet.
  Node::IntersectionInfo* intersection_info_ = nullptr;
};

class GlobalHitTester : public HitTester {
 public:
  GlobalHitTester() = default;
  virtual ~GlobalHitTester() = default;

 private:
  // A node always participates in the hit test, regardless of tag and session.
  bool should_participate(Node* node) override { return true; }
};

// Takes a distance-sorted list of hits, and if there are distance collisions in the list returns a
// warning message to be piped to FXL_LOG. If there are no collision, returns an empty string.
std::string GetDistanceCollisionsWarning(const std::vector<Hit>& hits);

}  // namespace gfx
}  // namespace scenic_impl

#endif  // SRC_UI_SCENIC_LIB_GFX_ENGINE_HIT_TESTER_H_
