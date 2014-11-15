#include "RenderConfig.hpp"

using srp::RenderState;

RenderState::RenderState(const RenderState & Other) {
  _threshold = Other._threshold;
  _current_shader = Other._current_shader;
}

RenderState & RenderState::operator= (const RenderState & Other) {
  this->_threshold = Other._threshold;
  this->_current_shader = Other._current_shader;
  return *this;
}

std::ostream & operator<< (std::ostream & Stream, const srp::RenderState & RS) {
  Stream << std::dec;
  Stream << "[RenderState: {Threshold : " << RS.GetThreshold()
                       << ",CurrentShader: " << *(RS.GetCurrentShaderProgram())
                       << "}]";
  return Stream;
}
