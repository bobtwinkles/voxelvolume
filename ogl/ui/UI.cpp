#include "UI.hpp"

static srp::XWindow * window;
static srp::ogl::ShaderProgram * graph;

void srp::ogl::ui::UIInit(srp::XWindow & Window) {
  window = &Window;
  graph = srp::ogl::CreateShader("graph.vert", "graph.frag");
}

srp::XWindow * srp::ogl::ui::UIGetWindow() {
  return window;
}

srp::ogl::ShaderProgram * srp::ogl::ui::GetGraphShader() {
  return graph;
}
