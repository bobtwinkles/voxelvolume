#ifndef _OGL_VERTEX_BUFFER_H_
#define _OGL_VERTEX_BUFFER_H_

#include <GL/glew.h>
#include <vector>

#include "Util.hpp"
#include "RenderConfig.hpp"

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
        GLuint _index_buffer;
        GLuint _data_buffer;
        GLuint _vao;
        GLenum _usage;
        GLenum _mode;

        std::vector<Vertex> _verticies;
        std::vector<GLuint> _indicies;

        bool _up_to_date;

        void _Init();

        DISALLOW_COPY_AND_ASSIGN(VertexBuffer);
      public:
        VertexBuffer(GLenum Mode, GLenum Usage);
        VertexBuffer(std::vector<Vertex> Verticies, std::vector<GLuint> Indices, GLenum Mode, GLenum Usage);
        ~VertexBuffer();

        VertexBuffer * AddVertex(Vertex);
        void ReplaceData(std::vector<Vertex> & Verticies, std::vector<GLuint> Indices);

        void Sync(); // Syncs data to the GPU

        void Render(srp::RenderState State) const;
    };
  }
}

std::ostream & operator<< (std::ostream & Stream, const srp::ogl::Vertex & Vert);

#endif // _OGL_VERTEX_BUFFER_H_
