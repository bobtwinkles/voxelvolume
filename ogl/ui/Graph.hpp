#ifndef _OGL_UI_GRAPH_
#define _OGL_UI_GRAPH_

#include <GL/glew.h>

#include "Util.hpp"
#include "RenderConfig.hpp"
#include "UI.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace srp {
  namespace ogl {
    namespace ui {
      template<typename T>
      class Graph {
        private:
          struct Vertex {
            float x, y;
            float r, g, b, a;
          };

          // Data spec
          T * _data_source;
          unsigned int _num_points;

          // Opengl stuff
          GLuint _data_buffer;
          GLuint _vao;

          // render data
          Vertex * _verts;

          // view spec
          float _x, _y, _width, _height;

          DISALLOW_COPY_AND_ASSIGN(Graph);
        public:
          Graph(T * DataBacking, unsigned int NumPoints, float X, float Y, float Width, float Height) {
            _data_source = DataBacking;
            _num_points = NumPoints;

            glGenVertexArrays(1, &_vao);
            glGenBuffers(1, &_data_buffer);

            _x = X;
            _y = Y;
            _width = Width;
            _height = Height;
            _verts = new Vertex[_num_points];
          }

          ~Graph() {
            glDeleteBuffers(1, &_data_buffer);
            glDeleteVertexArrays(1, &_vao);
          }

          void Render(srp::RenderState & State, T Min, T Max) const {
            unsigned int w, h;
            for (auto i = 0; i < _num_points; ++i) {
              _verts[i].x = _x + (i / float(_num_points)) * _width;
              _verts[i].y = _y + (float(_data_source[i] - Min) / float(Max)) * _height;
              _verts[i].r = 1;
              _verts[i].g = 1;
              _verts[i].b = 1;
            }

            glBindVertexArray(_vao);
            glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
            glBufferData(GL_ARRAY_BUFFER, _num_points * sizeof(Vertex), _verts, GL_DYNAMIC_DRAW);

            UIGetWindow()->GetGeometry(&w, &h);
            glm::mat4 trans = glm::ortho(0.f, (float)w, 0.f, (float)h, 10.f, -10.f);

            srp::ogl::ShaderProgram * cs = State.GetCurrentShaderProgram();

            if (cs != GetGraphShader()) {
              cs = GetGraphShader();
              cs->Bind();
              State.SetCurrentShader(cs);
            }

            GLint position = cs->GetAttributeLocation("position");
            GLint color    = cs->GetAttributeLocation("color");

            cs->Upload("transform", trans);
            if (position >= 0) {
              glEnableVertexAttribArray(position);
              glVertexAttribPointer(position, 2, GL_FLOAT, false, sizeof(Vertex), 0);
            }
            if (color >= 0) {
              glEnableVertexAttribArray(color);
              glVertexAttribPointer(color, 4, GL_FLOAT, false, sizeof(Vertex), (void*)(2 * sizeof(float)));
            }

            glDrawArrays(GL_LINE_STRIP, 0, _num_points);

            if (position >= 0) {
              glDisableVertexAttribArray(position);
            }
            if (color >= 0) {
              glDisableVertexAttribArray(color);
            }
          }
      };
    }
  }
}

#endif // _OGL_UI_GRAPH_
