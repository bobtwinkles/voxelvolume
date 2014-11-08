#ifndef _OGL_VERTEX_BUFFER_H_
#define _OGL_VERTEX_BUFFER_H_

#include <GL/glew.h>
#include <vector>

#include "Util.hpp"

namespace srp {
  namespace ogl {
    struct Vertex {
      float x, y, z;
      float r, g, b;
      float nx, ny, nz;
      float u, v;
    };

    class VertexBuffer {
      private:
        GLint _index_buffer;
        GLint _data_buffer;
      public:
        VertexBuffer(std::vector<Vertex> Verticies, std::vector<GLuint> Indices, GLenum Usage);
        ~VertexBuffer();

        void Render() const;
    };
  }
}

#endif // _OGL_VERTEX_BUFFER_H_
