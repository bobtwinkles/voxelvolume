#ifndef _RENDER_CHUNK_H_
#define _RENDER_CHUNK_H_

#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "Tetrahedron.hpp"
#include "Util.hpp"

#include <vector>

namespace srp {
  class RenderChunk {
    private:
      typedef std::vector<srp::Tetrahedron> TetrahedronList;

      srp::DataStore & _ds;

      int _x, _y, _z;

      std::vector<srp::Tetrahedron> _tetrahedrons;

      DISALLOW_COPY_AND_ASSIGN(RenderChunk);
    public:
      // X, Y, and Z are in absolute coordinates
      RenderChunk(srp::DataStore & store, int X, int Y, int Z);
      ~RenderChunk();

      void Render(srp::RenderState & State);
  };
}

#endif
