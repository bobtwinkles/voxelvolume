#ifndef _RENDER_CONFIG_H_
#define _RENDER_CONFIG_H_

#include "Util.hpp"

// Global renderer state

namespace srp {
  class RenderState {
    private:
      DISALLOW_COPY_AND_ASSIGN(RenderState);
      unsigned int _threshold;
    public:
      RenderState(unsigned int Threshold) : _threshold(Threshold) {}
      ~RenderState() {}

      inline void SetThreshold(unsigned int Threshold) { this->_threshold = Threshold; }
      inline unsigned int GetThreshold(void) { return this->_threshold; }
  };
}

#endif
