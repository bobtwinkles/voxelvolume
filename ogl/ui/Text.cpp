#include "Text.hpp"

#include "Shader.hpp"
#include "OGLUtil.hpp"

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

static FT_Library library;
static FT_Face courier;
static srp::ogl::ShaderProgram * text_shader;
static srp::ogl::ShaderProgram * save;
// TODO: refactor this out into UI.cpp, for now it's duplicated
static srp::XWindow * window;
static bool rendering;

static GLuint texture;
static GLuint vao;
static GLuint data_buffer;

using namespace srp::ogl::ui;

void srp::ogl::ui::TextInit(srp::XWindow & Window) {
  int error;

  error = FT_Init_FreeType(&library);

  if (error) {
    std::cerr << "Failed to initialize freetype, error " << error << std::endl;
    BUG();
  }

  if (FT_New_Face(library, "/usr/share/fonts/dejavu/DejaVuSansMono.ttf", 0, &courier)) {
    std::cerr << "Failed to open cour.tff" << std::endl;
    BUG();
  }

  // 12 pt. at 72 dpi
  FT_Set_Char_Size(courier, 12 * 64, 0, 72, 72);

  std::cout << "We have " << courier->num_glyphs << " glyphs avalible" << std::endl;

  window = &Window;

  text_shader = srp::ogl::CreateShader("text.vert", "text.frag");

  rendering = false;

  glGenTextures(1, &texture);
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &data_buffer);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void srp::ogl::ui::TextDrawBegin(srp::RenderState & State) {
  save = State.GetCurrentShaderProgram();
  State.SetCurrentShader(text_shader);
  text_shader->Bind();

  rendering = true;

  TextDrawColor(1, 1, 1);
}

void srp::ogl::ui::TextDrawColor(float R, float G, float B) {
  if (!rendering) {
    std::cerr << "tried to set text color outside of text rending mode!" << std::endl
              << "You should call TextDrawBegin/TextDrawEnd" << std::endl;
    BUG();
  }
  text_shader->Upload("color", glm::vec4(R, G, B, 1));
}

void srp::ogl::ui::TextDrawString(int X, int Y, const char * String) {
  glm::mat4 trans;
  const char * p;
  unsigned int w, h;
  FT_GlyphSlot g = courier->glyph;

  if (!rendering) {
    std::cerr << "tried to draw text outside of text rending mode!" << std::endl
              << "You should call TextDrawBegin/TextDrawEnd" << std::endl;
    BUG();
  }

  glBindVertexArray(vao);

  window->GetGeometry(&w, &h);
  trans = glm::ortho(0.f, (float)w, 0.f, (float)h, 10.f, -10.f);

  text_shader->Upload("transform", trans);

  glBindBuffer(GL_ARRAY_BUFFER, data_buffer);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  text_shader->Upload("texture", 0);

  GLint coord = text_shader->GetAttributeLocation("coordinate");
  if (coord >= 0) {
    glEnableVertexAttribArray(coord);
    glVertexAttribPointer(coord, 4, GL_FLOAT, false, 4 * sizeof(GLfloat), 0);
  }

  GLERR();

  for (p = String; *p; p++) {
    if (FT_Load_Char(courier, *p, FT_LOAD_RENDER)) {
      continue;
    }
    g = courier->glyph;

    if (!(g->bitmap.buffer)) {
      X += (g->advance.x >> 6);
      Y += (g->advance.y >> 6);
      continue;
    }

    glTexImage2D(
              GL_TEXTURE_2D,
              0,
              GL_R8,
              g->bitmap.width,
              g->bitmap.rows,
              0,
              GL_RED,
              GL_UNSIGNED_BYTE,
              g->bitmap.buffer
    );
    GLERR();

    int x2 = X + g->bitmap_left;
    int y2 = Y + g->bitmap_top;
    int w = g->bitmap.width;
    int h = g->bitmap.rows;

    GLfloat box[4][4] = {
        { (GLfloat)(x2    ), (GLfloat)(y2    ), 0, 0},
        { (GLfloat)(x2 + w), (GLfloat)(y2    ), 1, 0},
        { (GLfloat)(x2    ), (GLfloat)(y2 - h), 0, 1},
        { (GLfloat)(x2 + w), (GLfloat)(y2 - h), 1, 1},
    };

    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), box, GL_DYNAMIC_DRAW);
    GLERR();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    X += (g->advance.x >> 6);
    Y += (g->advance.y >> 6);

    GLERR();
  }

  if (coord >= 0) {
    glDisableVertexAttribArray(coord);
  }

  GLERR();
}

void srp::ogl::ui::TextDrawEnd(srp::RenderState & State) {
  State.SetCurrentShader(save);
  if (save != 0) {
    save->Bind();
  }
  GLERR();
  rendering = false;
}
