// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/ui/scenic/cpp/view_ref_pair.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>

#include "src/ui/scenic/lib/gfx/tests/gfx_test.h"
#include "src/ui/scenic/lib/gfx/tests/mocks/util.h"
#include "src/ui/scenic/lib/utils/helpers.h"

namespace scenic_impl::gfx::test {

// Class fixture for TEST_F.
class ViewTreeIntegrationTest : public scenic_impl::gfx::test::GfxSystemTest {
 protected:
  ViewTreeIntegrationTest() = default;
  ~ViewTreeIntegrationTest() override = default;

  void RequestToPresent(scenic::Session* session) {
    session->Present(/*presentation time*/ 0, [](auto) {});
    RunLoopFor(kWaitTime);
  }

  const ViewTree& view_tree() { return engine()->scene_graph()->view_tree(); }

  // "Good enough" deadline to ensure session update gets scheduled.
  const zx::duration kWaitTime = zx::msec(20);
};

// Sets up a basic scene where View of Session B is connected to Scene root of Session A:
//     A
//     |
//     B
// Check that we don't require both Session A and B to have updates scheduled the frame
// when the View-ViewHolder connection completes.
TEST_F(ViewTreeIntegrationTest, ViewsConnectedWithoutScheduledUpdates_ShouldSeeViewTreeUpdates) {
  auto [view_token, view_holder_token] = scenic::ViewTokenPair::New();

  auto [control_ref_b, view_ref_b] = scenic::ViewRefPair::New();
  const zx_koid_t view_ref_koid_b = utils::ExtractKoid(view_ref_b);
  EXPECT_FALSE(view_tree().IsTracked(view_ref_koid_b));

  // Set up client B (the child) first.
  SessionWrapper client_b(scenic());
  const scenic::View view(client_b.session(), std::move(view_token), std::move(control_ref_b),
                          std::move(view_ref_b), "view");
  RequestToPresent(client_b.session());

  // View hasn't been connected to ViewHolder, so it shouldn't be connected in the ViewTree.
  EXPECT_TRUE(view_tree().IsTracked(view_ref_koid_b));
  EXPECT_FALSE(view_tree().IsConnectedToScene(view_ref_koid_b));

  // Set up a minimal scene in client A;
  SessionWrapper client_a(scenic());

  scenic::Scene scene(client_a.session());
  scenic::Camera camera(scene);
  scenic::Renderer renderer(client_a.session());
  renderer.SetCamera(camera);
  scenic::Layer layer(client_a.session());
  layer.SetRenderer(renderer);
  scenic::LayerStack layer_stack(client_a.session());
  layer_stack.AddLayer(layer);
  scenic::Compositor compositor(client_a.session());
  compositor.SetLayerStack(layer_stack);

  // Attach the ViewHolder to the root node.
  scenic::ViewHolder view_holder(client_a.session(), std::move(view_holder_token), "view holder");
  scene.AddChild(view_holder);

  // When presenting this update client B should have no scheduled updates. But we still expect the
  // ViewTree to be updated correctly.
  RequestToPresent(client_a.session());
  EXPECT_TRUE(view_tree().IsConnectedToScene(view_ref_koid_b));
}

// Sets up a basic scene where View of Session B is connected to Scene root of Session A:
//     A
//     |
//     B
// Then destroys Session B and checks that View B is correctly removed from the ViewTree.
TEST_F(ViewTreeIntegrationTest, SessionDeath_ShouldTriggerViewTreeUpdates) {
  auto [view_token, view_holder_token] = scenic::ViewTokenPair::New();

  // Set up a minimal scene in client A;
  SessionWrapper client_a(scenic());

  scenic::Scene scene(client_a.session());
  scenic::Camera camera(scene);
  scenic::Renderer renderer(client_a.session());
  renderer.SetCamera(camera);
  scenic::Layer layer(client_a.session());
  layer.SetRenderer(renderer);
  scenic::LayerStack layer_stack(client_a.session());
  layer_stack.AddLayer(layer);
  scenic::Compositor compositor(client_a.session());
  compositor.SetLayerStack(layer_stack);

  // Attach the ViewHolder to the root node.
  scenic::ViewHolder view_holder(client_a.session(), std::move(view_holder_token), "view holder");
  scene.AddChild(view_holder);
  RequestToPresent(client_a.session());

  auto [control_ref_b, view_ref_b] = scenic::ViewRefPair::New();
  const zx_koid_t view_ref_koid_b = utils::ExtractKoid(view_ref_b);
  {  // Set up client B.
    SessionWrapper client_b(scenic());
    EXPECT_FALSE(view_tree().IsTracked(view_ref_koid_b));

    const scenic::View view(client_b.session(), std::move(view_token), std::move(control_ref_b),
                            std::move(view_ref_b), "view");

    // When both clients have presented, we should see View B connected in the ViewTree.
    RequestToPresent(client_b.session());
    EXPECT_TRUE(view_tree().IsTracked(view_ref_koid_b));
    EXPECT_TRUE(view_tree().IsConnectedToScene(view_ref_koid_b));
  }  // B goes out of scope. Observe that an update is scheduled that removes B from the ViewTree.

  EXPECT_TRUE(view_tree().IsTracked(view_ref_koid_b));
  RunLoopFor(kWaitTime);  // Wait long enough for an update to be applied.
  EXPECT_FALSE(view_tree().IsTracked(view_ref_koid_b));
}

}  // namespace scenic_impl::gfx::test
