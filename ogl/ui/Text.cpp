#include "Text.hpp"

#include "Shader.hpp"
#include "OGLUtil.hpp"

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fontconfig/fontconfig.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>

struct char_spec {
  glm::vec2 tl;
  glm::vec2 br;
  int x_advance;
  int y_advance;
  int bm_width;
  int bm_height;
  int bm_left;
  int bm_top;
};

static FT_Library library;
static FT_Face freetype_font;
static srp::ogl::ShaderProgram * text_shader;
static srp::ogl::ShaderProgram * save;
// TODO: refactor this out into UI.cpp, for now it's duplicated
static srp::XWindow * window;
static bool rendering;

static std::unordered_map<char, char_spec> char_cache;

static GLuint texture;
static GLuint vao;
static GLuint data_buffer;

using namespace srp::ogl::ui;

#define CACHE_TEXTURE_SIZE 128

void srp::ogl::ui::TextInit(srp::XWindow & Window) {
  FcResult result;
  char * file;
  int error;

  error = FT_Init_FreeType(&library);

  if (error) {
    std::cerr << "Failed to initialize freetype, error " << error << std::endl;
    BUG();
  }

  // Load the default fontconfig configuration
  if (!FcInit()) {
    std::cerr << "Failed to initialize fontconfig, cannot find font";
    BUG();
  }
  // We want a 12 point fixed width font
  FcPattern * pattern = FcPatternCreate(); //FcNameParse((const FcChar8*)":spacing=100:width=80"); //:style=regular:scalable=False");
  FcPatternAddInteger(pattern, "spacing", FC_MONO);
  FcPatternAddInteger(pattern, "weight", 80);
  FcPatternAddInteger(pattern, "slant", 0);
  FcPatternPrint(pattern);

  FcObjectSet * os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, (char *) 0);

  FcFontSet * fs = FcFontList(0, pattern, os);

  FcPatternDestroy(pattern);
  FcObjectSetDestroy(os);

  std::cout << "There are " << fs->nfont << " font matches" << std::endl;
  //Actually find the font
  bool font_found = false;
  if (fs) {
    for (int j = 0; j < fs->nfont; ++j) {
      if (FcPatternGetString(fs->fonts[j], FC_FILE, 0, (FcChar8 **)&file) != FcResultMatch) {
        file = (char*)"<unknown filename>";
      } else {
        std::cout << "checking " << file << std::endl;
        if (FT_New_Face(library, (const char *)file, 0, &freetype_font)) {
          std::cerr << "Failed to open " << file << std::endl;
        } else {
          if (freetype_font->num_glyphs > 100) {
            std::cout << "Will use " << file << " as the font." << std::endl;
            font_found = true;
            break;
          }
        }
      }
    }
  }

  if (!font_found) {
    std::cerr << "Failed to find a font" << std::endl;
    BUG();
  }

  // Destroy the font set after we're done with the file name
  FcFontSetDestroy(fs);

  // 12 pt. at 72 dpi
  FT_Set_Char_Size(freetype_font, 12 * 64, 0, 72, 72);

  std::cout << "We have " << freetype_font->num_glyphs << " glyphs avalible" << std::endl;

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

  FT_GlyphSlot g = freetype_font->glyph;
  int cx, cy;
  cx = cy = 0;
  int max_height = 0;
  unsigned char * buffer = new unsigned char [CACHE_TEXTURE_SIZE * CACHE_TEXTURE_SIZE];
  for (char i = 20; i < 126; ++i) {
    if (FT_Load_Char(freetype_font, i, FT_LOAD_RENDER)) {
      std::cerr << "WARNING: Failed to load character " << i << std::endl;
      continue;
    }
    char_spec s;
    s.bm_top = g->bitmap_top;
    s.bm_left = g->bitmap_left;
    s.bm_width = g->bitmap.width;
    s.bm_height = g->bitmap.rows;
    s.x_advance = g->advance.x >> 6;
    s.y_advance = g->advance.y >> 6;
    s.tl = glm::vec2(cx / float(CACHE_TEXTURE_SIZE), cy / float(CACHE_TEXTURE_SIZE));
    s.br = glm::vec2((cx + s.bm_width) / float(CACHE_TEXTURE_SIZE), (cy + s.bm_height) / float(CACHE_TEXTURE_SIZE));
    if (s.bm_height > max_height) {
      max_height = g->bitmap.rows;
    }
    if (s.bm_width + cx + 1 >= CACHE_TEXTURE_SIZE) {
      cx = 0;
      cy += max_height + 2;
      s.tl = glm::vec2(cx / float(CACHE_TEXTURE_SIZE), cy / float(CACHE_TEXTURE_SIZE));
      s.br = glm::vec2((cx + s.bm_width) / float(CACHE_TEXTURE_SIZE), (cy + s.bm_height) / float(CACHE_TEXTURE_SIZE));
      if (cy > CACHE_TEXTURE_SIZE) {
        std::cerr << "ERROR: Ran out of space in the font cache texture" << std::endl;
        BUG();
      }
    }
    char_cache.insert(std::make_pair(i, s));
    if (!(g->bitmap.buffer)) {
      cx += s.bm_width + 1;
      continue;
    }
    for (int x = 0; x < s.bm_width; ++x) {
      for (int y = 0; y < s.bm_height; ++y) {
        buffer[(x + cx) + CACHE_TEXTURE_SIZE * (y + cy)] = g->bitmap.buffer[x + y * s.bm_width];
      }
    }
    cx += s.bm_width + 1;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, CACHE_TEXTURE_SIZE, CACHE_TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
  delete[] buffer;

  glBindTexture(GL_TEXTURE_2D, 0);
  GLERR();
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
  FT_GlyphSlot g = freetype_font->glyph;

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
    char_spec s = char_cache.find(*p)->second;

    int x2 = X + s.bm_left;
    int y2 = Y + s.bm_top;
    int w = s.bm_width;
    int h = s.bm_height;

    GLfloat box[4][4] = {
        { (GLfloat)(x2    ), (GLfloat)(y2    ), s.tl.x, s.tl.y},
        { (GLfloat)(x2 + w), (GLfloat)(y2    ), s.br.x, s.tl.y},
        { (GLfloat)(x2    ), (GLfloat)(y2 - h), s.tl.x, s.br.y},
        { (GLfloat)(x2 + w), (GLfloat)(y2 - h), s.br.x, s.br.y},
    };

    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), box, GL_DYNAMIC_DRAW);
    GLERR();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    X += s.x_advance;
    Y += s.y_advance;

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
