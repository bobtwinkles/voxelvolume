#include "VertexBuffer.hpp"

#include "OGLUtil.hpp"

using srp::ogl::VertexBuffer;
using srp::ogl::Vertex;

#define OFFSET(a) ((void*)((char*)NULL + ((a)*sizeof(float))))

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

    std::cout << "There are " << _indicies.size() << " indicies to transfer" << std::endl;

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

  if (State.GetPositionIndex() >= 0) {
    glEnableVertexAttribArray(State.GetPositionIndex());
    glVertexAttribPointer(State.GetPositionIndex(), 3, GL_FLOAT, false, sizeof(Vertex), 0);
    GLERR();
  }
  if (State.GetColorIndex() >= 0) {
    glEnableVertexAttribArray(State.GetColorIndex());
    glVertexAttribPointer(State.GetColorIndex()   , 3, GL_FLOAT, false, sizeof(Vertex), OFFSET(3));
    GLERR();
  }
  GLERR();

  glDrawElements(_mode, _indicies.size(), GL_UNSIGNED_INT, 0);
  GLERR();

  if (State.GetPositionIndex() >= 0) {
    glDisableVertexAttribArray(State.GetPositionIndex());
    GLERR();
  }
  if (State.GetColorIndex() >= 0) {
    glDisableVertexAttribArray(State.GetColorIndex());
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
