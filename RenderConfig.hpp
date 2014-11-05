#ifndef _RENDER_CONFIG_H_
#define _RENDER_CONFIG_H_

#include "Util.hpp"

// Global renderer state

#define RENDER_CHUNK_SIZE 32

namespace srp {
  struct Vertex {
    float x, y, z;
    float r, g, b;
    float nx, ny, nz;
    float u, v;
  };

  class RenderState {
    private:
      unsigned int _threshold;
    public:
      RenderState(unsigned int Threshold) : _threshold(Threshold) {}
      RenderState(const RenderState & Other) : _threshold(Other._threshold) {}
      ~RenderState() {}

      inline void SetThreshold(unsigned int Threshold) { this->_threshold = Threshold; }
      inline unsigned int GetThreshold(void) const { return this->_threshold; }

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
