#ifndef _OGL_TEXTURED_VERTEX_BUFFER_
#define _OGL_TEXTURED_VERTEX_BUFFER_

#include <GL/glew.h>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include "Util.hpp"
#include "RenderConfig.hpp"

namespace srp {
  namespace ogl {
    struct TexturedVertex {
      float x, y, z, w;
      float u, v;
    };

    class TexturedVertexBuffer {
      private:
        GLuint _index_buffer;
        GLuint _data_buffer;
        GLuint _vao;
        GLuint _texture;
        GLenum _usage;
        GLenum _mode;

        std::vector<TexturedVertex> _verticies;
        std::vector<GLuint> _indicies;

        bool _verts_ok;

        void _Init();

        DISALLOW_COPY_AND_ASSIGN(TexturedVertexBuffer);
      public:
        TexturedVertexBuffer(GLenum Mode, GLenum Usage);
        ~TexturedVertexBuffer();

        void AddVertex(TexturedVertex Vertex);
        void AddVerticies(std::vector<TexturedVertex> Verts, std::vector<GLuint> Inidicies);
        void ClearVertexData();

        void SetTextureData(GLsizei Width, GLsizei Height, GLenum Format, GLenum Type, GLvoid * data);

        void Sync();

        void Render(const srp::RenderState & State) const;
    };

    void QueueTexturedRectangle(TexturedVertexBuffer & Buffer, glm::vec3 PStart, glm::vec3 PEnd
                                                             , glm::vec2 TStart, glm::vec2 TEnd);
  }
}

#endif // _OGL_TEXTURED_VERTEX_BUFFER_
