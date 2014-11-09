#include "RenderConfig.hpp"

using srp::RenderState;

RenderState::RenderState(const RenderState & Other) {
  _threshold = Other._threshold;
  _position_index = Other._position_index;
  _color_index = Other._color_index;
}

RenderState & RenderState::operator= (const RenderState & Other) {
  this->_threshold = Other._threshold;
  this->_position_index = Other._position_index;
  this->_color_index = Other._color_index;
  return *this;
}

std::ostream & operator<< (std::ostream & Stream, const srp::RenderState & RS) {
  Stream << std::dec;
  Stream << "[RenderState: {Threshold : " << RS.GetThreshold()
                       << ",PositionIndex: " << RS.GetPositionIndex()
                       << ",ColorIndex: " << RS.GetColorIndex()
                       << "}]";
  return Stream;
}
