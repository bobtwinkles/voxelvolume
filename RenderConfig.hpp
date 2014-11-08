#ifndef _RENDER_CONFIG_H_
#define _RENDER_CONFIG_H_

#include <GL/glew.h>

#include "Util.hpp"

// Global renderer state

#define RENDER_CHUNK_SIZE 32

namespace srp {
  class RenderState {
    private:
      unsigned int _threshold;
      GLint _position_index, _color_index;
    public:
      RenderState(unsigned int Threshold) : _threshold(Threshold), _position_index(-1), _color_index(-1) {}
      RenderState(const RenderState & Other) : _threshold(Other._threshold) {}
      ~RenderState() {}

      inline void SetThreshold(unsigned int Threshold) { this->_threshold = Threshold; }
      inline unsigned int GetThreshold(void) const { return this->_threshold; }

      inline void SetPositionIndex(GLint PI) { _position_index = PI; }
      inline GLint GetPositionIndex() { return _position_index; }

      inline void SetColorIndex(GLint PI) { _color_index = PI; }
      inline GLint GetColorIndex() { return _color_index; }

      RenderState & operator= (const RenderState & Other) {
        this->_threshold = Other._threshold;
        return *this;
      }

      inline bool operator==(const RenderState & Other) const {
        if (&Other == this) {
          return true;
        }
        return Other._threshold == this->_threshold;
      }

      inline bool operator!=(const RenderState & Other) const {
        return !(*this == Other);
      }
  };
}

#endif
