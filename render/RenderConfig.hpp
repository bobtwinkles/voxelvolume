#ifndef _RENDER_CONFIG_H_
#define _RENDER_CONFIG_H_

#include <GL/glew.h>

#include "Util.hpp"
#include "ogl/Shader.hpp"

// Global renderer state

#define RENDER_CHUNK_SIZE 32

namespace srp {
  class RenderState {
    private:
      unsigned int _threshold;
      GLint _position_index, _color_index;
      srp::ogl::ShaderProgram * _current_shader;
    public:
      RenderState(unsigned int Threshold) : _threshold(Threshold), _current_shader(NULL) {}
      RenderState(const RenderState & Other);
      ~RenderState() {}

      inline void SetThreshold(unsigned int Threshold) { this->_threshold = Threshold; }
      inline unsigned int GetThreshold(void) const { return this->_threshold; }

      inline void SetCurrentShader(srp::ogl::ShaderProgram * SP) { _current_shader = SP; }
      inline srp::ogl::ShaderProgram * GetCurrentShaderProgram() const { return _current_shader; }

      RenderState & operator= (const RenderState & Other);

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

std::ostream & operator<< (std::ostream & Stream, const srp::RenderState & RS);

#endif
