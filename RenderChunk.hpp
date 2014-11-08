#ifndef _RENDER_CHUNK_H_
#define _RENDER_CHUNK_H_

#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "Tetrahedron.hpp"
#include "Util.hpp"
#include "ogl/VertexBuffer.hpp"

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>

namespace srp {
  class RenderChunk {
    private:
      typedef std::vector<srp::Tetrahedron> TetrahedronList;

      srp::DataStore & _ds;

      int _x, _y, _z;

      GLuint _vertex_buffer;
      GLuint _index_buffer;
      GLuint _vao;

      int _num_verts;

      std::vector<srp::Tetrahedron> _tetrahedrons;

      srp::RenderState _last_state;

      void _UpdateBuffers(std::vector<srp::ogl::Vertex> &, std::vector<GLuint> &);

      DISALLOW_COPY_AND_ASSIGN(RenderChunk);
    public:
      // X, Y, and Z are in chunk coordinates
      RenderChunk(srp::DataStore & store, int X, int Y, int Z);
      ~RenderChunk();

      void Update(srp::RenderState & State);
      void Render(srp::RenderState & State);
  };
}

#endif
