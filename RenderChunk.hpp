#ifndef _RENDER_CHUNK_H_
#define _RENDER_CHUNK_H_

#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "Util.hpp"

#define RENDER_CHUNK_SIZE 16

namespace srp {
  class RenderChunk {
    private:
      srp::DataStore & _ds;

      int _x, _y, _z;

      DISALLOW_COPY_AND_ASSIGN(RenderChunk);
    public:
      // X, Y, and Z are in absolute coordinates
      RenderChunk(srp::DataStore & store, int X, int Y, int Z);
      ~RenderChunk();

      void Render(srp::RenderState & State);
  };
}

#endif
