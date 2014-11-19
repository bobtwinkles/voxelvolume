#include "VertexBuffer.hpp"

#include "OGLUtil.hpp"

using srp::ogl::VertexBuffer;
using srp::ogl::Vertex;

void VertexBuffer::_Init() {
  GLuint buffers[2];
  glGenBuffers(2, buffers);
  _index_buffer = buffers[0];
  _data_buffer  = buffers[1];
  glGenVertexArrays(1, &_vao);
  GLERR();
}

VertexBuffer::VertexBuffer(GLenum Mode, GLenum Usage) {
  _Init();
  _usage = Usage;
  _mode = Mode;
  _up_to_date = false;
}

VertexBuffer::VertexBuffer(std::vector<Vertex> Verticies, std::vector<GLuint> Indices, GLenum Mode, GLenum Usage) {
  _Init();
  _up_to_date = false;
  _usage = Usage;
  _mode = Mode;
  ReplaceData(Verticies, Indices);
  _verticies.insert(Verticies.end(), Verticies.begin(), Verticies.end());
  _indicies.insert(Indices.end(), Indices.begin(), Indices.end());
}

VertexBuffer::~VertexBuffer() {
  GLuint buffers[2];
  buffers[0] = _index_buffer;
  buffers[1] = _data_buffer;
  glDeleteBuffers(2, buffers);
  glDeleteVertexArrays(1, &_vao);
  std::cerr << "Deleted VertexBuffer" << std::endl;
}

VertexBuffer * VertexBuffer::AddVertex(Vertex V) {
  std::cout << "Adding vertex " << V << std::endl;
  _indicies.push_back((GLuint)(_verticies.size()));
  _verticies.push_back(V);
  _up_to_date = false;

  return this;
}

void VertexBuffer::ReplaceData(std::vector<Vertex> & Verticies, std::vector<GLuint> Indices) {
  _verticies = Verticies;
  _indicies = Indices;

  //_indicies.clear();
  //_verticies.clear();

  //_verticies.insert(_verticies.end(), Verticies.begin(), Verticies.end());
  //_indicies.insert(_indicies.end(), Indices.begin(), Indices.end());

  _up_to_date = false;
}

void VertexBuffer::Sync() {
  if (!_up_to_date) {

    glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer);
    GLERR();

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _verticies.size(), _verticies.data(), _usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _indicies.size(), _indicies.data(), _usage);
    GLERR();
    _up_to_date = true;
  }
}

void VertexBuffer::Render(srp::RenderState State) const {
  if (!_up_to_date) {
    std::cerr << "Tried to render non-synced VertexBuffer!" << std::endl;
    BUG();
  }
  glBindVertexArray(_vao);
  glBindBuffer(GL_ARRAY_BUFFER, _data_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _index_buffer);
  GLERR();

  srp::ogl::ShaderProgram * cs = State.GetCurrentShaderProgram();
  GLint pos_index = cs->GetAttributeLocation("position");
  GLint color_index = cs->GetAttributeLocation("color");
  GLint normal_index = cs->GetAttributeLocation("normal");

  if (pos_index >= 0) {
    glEnableVertexAttribArray(pos_index);
    glVertexAttribPointer    (pos_index  , 3, GL_FLOAT, false, sizeof(Vertex), 0);
    GLERR();
  }
  if (color_index >= 0) {
    glEnableVertexAttribArray(color_index);
    glVertexAttribPointer    (color_index, 3, GL_FLOAT, false, sizeof(Vertex), OFFSET(4));
    GLERR();
  }
  if (normal_index >= 0) {
    glEnableVertexAttribArray(normal_index);
    glVertexAttribPointer    (normal_index, 3, GL_FLOAT, false, sizeof(Vertex), OFFSET(8));
    GLERR();
  }

  glDrawElements(_mode, _indicies.size(), GL_UNSIGNED_INT, 0);
  GLERR();

  if (pos_index >= 0) {
    glDisableVertexAttribArray(pos_index);
    GLERR();
  }
  if (color_index >= 0) {
    glDisableVertexAttribArray(color_index);
    GLERR();
  }
  if (normal_index >= 0) {
    glDisableVertexAttribArray(normal_index);
    GLERR();
  }

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::ostream & operator<< (std::ostream & Stream, const Vertex & Vert) {
  Stream << std::dec;
  Stream << "[Vertex: {Pos: (" << Vert.x <<
                           "," << Vert.y <<
                           "," << Vert.z <<
                //"), Normal: (" << Vert.nx <<
                //           "," << Vert.ny <<
                //           "," << Vert.nz <<
                 "), Color: (" << Vert.r <<
                           "," << Vert.g <<
                           "," << Vert.b <<
                //"), UV: (" << Vert.u << "," << Vert.v <<
                 "}]";
  return Stream;
}
