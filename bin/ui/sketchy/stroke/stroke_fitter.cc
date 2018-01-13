// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "garnet/bin/ui/sketchy/stroke/stroke_fitter.h"

#include "lib/fxl/logging.h"

namespace {

constexpr float kEpsilon = 6e-6;
constexpr float kErrorThreshold = 10;

inline std::ostream& operator<<(std::ostream& os, const sketchy::vec2& pt) {
  return os << "(" << pt.x << "," << pt.y << ")";
}

}  // namespace

namespace sketchy_service {

StrokeFitter::StrokeFitter(glm::vec2 start_pt)
    : path_(std::make_unique<StrokePath>()) {
  points_.push_back(start_pt);
  params_.push_back(0.f);
}

void StrokeFitter::Extend(const std::vector<glm::vec2>& sampled_pts) {
  bool changed = false;
  for (const auto& pt : sampled_pts) {
    float dist = glm::distance(pt, points_.back());
    if (dist > kEpsilon) {
      points_.push_back(pt);
      params_.push_back(params_.back() + dist);
      changed = true;
    }
  }
  if (!changed) {
    return;
  }

  // Recursively compute a list of cubic Bezier segments.
  // TODO: don't recompute stable path segments near the beginning of the
  // stroke.
  size_t end_index = points_.size() - 1;
  glm::vec2 left_tangent = points_[1] - points_[0];
  glm::vec2 right_tangent = points_[end_index - 1] - points_[end_index];
  FitSampleRange(0, static_cast<int>(end_index), left_tangent, right_tangent);
}

void StrokeFitter::FitSampleRange(int start_index, int end_index,
                                  glm::vec2 left_tangent,
                                  glm::vec2 right_tangent) {
  FXL_DCHECK(glm::length(left_tangent) > 0 && glm::length(right_tangent))
      << "  left: " << left_tangent << "  right: " << right_tangent;
  FXL_DCHECK(end_index > start_index);

  if (end_index - start_index == 1) {
    // Only two points... use a heuristic.
    // TODO: Double-check this heuristic (perhaps normalization needed?)
    // TODO: Perhaps this segment can be omitted entirely, e.g. by blending
    //       endpoints of the adjacent segments.
    sketchy::CubicBezier2f line;
    line.pts[0] = points_[start_index];
    line.pts[3] = points_[end_index];
    line.pts[1] = line.pts[0] + (left_tangent * 0.25f);
    line.pts[2] = line.pts[3] + (right_tangent * 0.25f);
    FXL_DCHECK(!std::isnan(line.pts[0].x));
    FXL_DCHECK(!std::isnan(line.pts[0].y));
    FXL_DCHECK(!std::isnan(line.pts[1].x));
    FXL_DCHECK(!std::isnan(line.pts[1].y));
    FXL_DCHECK(!std::isnan(line.pts[2].x));
    FXL_DCHECK(!std::isnan(line.pts[2].y));
    FXL_DCHECK(!std::isnan(line.pts[3].x));
    FXL_DCHECK(!std::isnan(line.pts[3].y));
    path_->ExtendWithCurve(line);
    return;
  }

  // Normalize cumulative length between 0.0 and 1.0.
  float param_shift = -params_[start_index];
  float param_scale = 1.0 / (params_[end_index] + param_shift);

  sketchy::CubicBezier2f curve =
      sketchy::FitCubicBezier2f(
          &(points_[start_index]), end_index - start_index + 1,
          &(params_[start_index]), param_shift, param_scale,
          left_tangent, right_tangent);

  int split_index = (end_index + start_index + 1) / 2;
  float max_error = 0.f;
  for (int i = start_index; i <= end_index; ++i) {
    float t = (params_[i] + param_shift) * param_scale;
    glm::vec2 diff = points_[i] - curve.Evaluate(t);
    float error = glm::dot(diff, diff);
    if (error > max_error) {
      max_error = error;
      split_index = i;
    }
  }

  // The current fit is good enough... add it to the path and stop recursion.
  if (max_error < kErrorThreshold) {
    FXL_DCHECK(!std::isnan(curve.pts[0].x));
    FXL_DCHECK(!std::isnan(curve.pts[0].y));
    FXL_DCHECK(!std::isnan(curve.pts[1].x));
    FXL_DCHECK(!std::isnan(curve.pts[1].y));
    FXL_DCHECK(!std::isnan(curve.pts[2].x));
    FXL_DCHECK(!std::isnan(curve.pts[2].y));
    FXL_DCHECK(!std::isnan(curve.pts[3].x));
    FXL_DCHECK(!std::isnan(curve.pts[3].y));
    path_->ExtendWithCurve(curve);
    return;
  }

  // Error is too large... split into two ranges and fit each.
  FXL_DCHECK(split_index > start_index && split_index < end_index);
  // Compute the tangent on each side of the split point.
  // TODO: some filtering may be desirable here.
  glm::vec2 right_middle_tangent =
      points_[split_index + 1] - points_[split_index];
  if (glm::length(right_middle_tangent) == 0.f) {
    // The two points on either side of the split point are identical: the
    // user's path doubled back upon itself.  Instead, compute the tangent using
    // the point at the split-index.
    right_middle_tangent = points_[split_index + 1] - points_[split_index];
  }
  glm::vec2 left_middle_tangent = right_middle_tangent * -1.f;
  FitSampleRange(start_index, split_index, left_tangent, left_middle_tangent);
  FitSampleRange(split_index, end_index, right_middle_tangent, right_tangent);
}

void StrokeFitter::Reset() {
  FXL_DCHECK(points_.size() > 0 && params_.size() > 0);
  points_.erase(points_.begin(), points_.begin() + points_.size() - 1);
  params_.erase(params_.begin(), params_.begin() + params_.size() - 1);
  path_->Reset();
}

}  // namespace sketchy_service
