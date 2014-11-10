#include "TexturedVertexBuffer.hpp"

#include "OGLUtil.hpp"

using srp::ogl::TexturedVertex;
using srp::ogl::TexturedVertexBuffer;

TexturedVertexBuffer::TexturedVertexBuffer(GLenum Mode, GLenum Usage) {
  _mode = Mode;
  _usage = Usage;

  _verts_ok = false;

  _Init();
}

void TexturedVertexBuffer::_Init() {
  GLuint buffers[2];

  glGenBuffers(2, buffers);
  glGenVertexArrays(1, &_vao);
  glGenTextures(1, &_texture);

  _index_buffer = buffers[0];
  _data_buffer  = buffers[1];
}

TexturedVertexBuffer::~TexturedVertexBuffer() {
  GLuint buffers[2];
  buffers[0] = _index_buffer;
  buffers[1] = _data_buffer;

  glDeleteBuffers(2, buffers);
  glDeleteVertexArrays(1, &_vao);
  glDeleteTextures(1, &_texture);
}

void TexturedVertexBuffer::AddVertex(TexturedVertex Vertex) {
  _indicies.push_back(_verticies.size());
  _verticies.push_back(Vertex);

  _verts_ok = false;
}

void TexturedVertexBuffer::ClearVertexData() {
  _indicies.clear();
  _verticies.clear();

  _verts_ok = false;
}

void TexturedVertexBuffer::AddVerticies(std::vector<TexturedVertex> Verts, std::vector<GLuint> Indicies) {
  GLuint base = _verticies.size();
  _verticies.insert(_verticies.end(), Verts.begin(), Verts.end());

  auto it = Indicies.begin();
  while (it != Indicies.end()) {
    _indicies.push_back(base + *it);
    ++it;
  }

  _verts_ok = false;
}

void TexturedVertexBuffer::SetTextureData(GLsizei Width, GLsizei Height, GLenum Format,
                                          GLenum Type, GLvoid * Data) {
  glBindTexture(GL_TEXTURE_2D, _texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, Format, Type, Data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}

void TexturedVertexBuffer::Sync() {
  glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer);

  glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * _verticies.size(), _verticies.data(), _usage);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _indicies.size(), _indicies.data(), _usage);

  _verts_ok = true;
}

void TexturedVertexBuffer::Render(const srp::RenderState & State) const {
  if (!_verts_ok) {
    std::cerr << "Tried to render non-synced TexturedVertexBuffer" << std::endl;
    BUG();
  }
  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer);
  GLERR();

  srp::ogl::ShaderProgram * cs = State.GetCurrentShaderProgram();
  GLint pos = cs->GetAttributeLocation("position");
  GLint tc  = cs->GetAttributeLocation("texcoord");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texture);
  cs->Upload("texture", 0); // we want to use texture 0

  if (!(pos < 0)) {
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, false, sizeof(TexturedVertex), 0);
  }

  if (!(tc < 0)) {
    glEnableVertexAttribArray(tc);
    glVertexAttribPointer(tc, 2, GL_FLOAT, false, sizeof(TexturedVertex), OFFSET(4));
  }

  glDrawElements(_mode, _indicies.size(), GL_UNSIGNED_INT, 0);

  if (!(pos < 0)) {
    glDisableVertexAttribArray(pos);
  }

  if (!(tc < 0)) {
    glDisableVertexAttribArray(tc);
  }

  GLERR();
}


//////////////////////////////////////////
//////////////////////////////////////////
/////////////// Utilities ////////////////
//////////////////////////////////////////
//////////////////////////////////////////

void srp::ogl::QueueTexturedRectangle(TexturedVertexBuffer & Buffer, glm::vec3 PStart, glm::vec3 PEnd
                                                                   , glm::vec2 TStart, glm::vec2 TEnd) {
  TexturedVertex vert;
  std::vector<TexturedVertex> tid;
  std::vector<GLuint> tii;
  vert.w = 0;

  vert.x = PStart.x;
  vert.y = PStart.y;
  vert.z = PStart.z;
  vert.u = TStart.x;
  vert.v = TStart.y;
  tid.push_back(vert);

  vert.y = PEnd.y;
  vert.v = TEnd.y;
  tid.push_back(vert);

  vert.x = PEnd.x;
  vert.y = PStart.y;
  vert.z = PEnd.z;
  vert.u = TEnd.x;
  vert.v = TStart.y;
  tid.push_back(vert);

  vert.y = PEnd.y;
  vert.v = TEnd.y;
  tid.push_back(vert);

  tii.push_back(0);
  tii.push_back(1);
  tii.push_back(2);
  tii.push_back(1);
  tii.push_back(2);
  tii.push_back(3);

  Buffer.AddVerticies(tid, tii);
}
