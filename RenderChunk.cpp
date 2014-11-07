#include "RenderChunk.hpp"
#include "RenderTypes.hpp"
#include "Tetrahedron.hpp"

#include <iostream>

using srp::RenderChunk;
using srp::Tetrahedron;
using srp::Vec3i;
using srp::Vertex;

RenderChunk::RenderChunk(srp::DataStore & ds, int X, int Y, int Z) : _ds(ds), _x(X), _y(Y), _z(Z), _last_state(0) {
  Tetrahedron * cube[6];
  for (int x = X * RENDER_CHUNK_SIZE; x < (X + 1) * RENDER_CHUNK_SIZE; ++x) {
    for (int y = Y * RENDER_CHUNK_SIZE; y < (Y + 1) * RENDER_CHUNK_SIZE; ++y) {
      for (int z = Z * RENDER_CHUNK_SIZE; z < (Z + 1) * RENDER_CHUNK_SIZE; ++z) {
        srp::BuildForCube(x, y, z, cube);
        for (int i = 0; i < 6; ++i) {
          this->_tetrahedrons.push_back(*cube[i]);
        }
      }
    }
  }

  GLuint buffers[2];
  glGenBuffers(2, buffers);

  this->_vertex_buffer = buffers[0];
  this->_index_buffer = buffers[1];
}

RenderChunk::~RenderChunk() {
  while (this->_tetrahedrons.size() > 0) {
    Tetrahedron t = this->_tetrahedrons.back();
    this->_tetrahedrons.pop_back();
    delete &t;
  }
  GLuint buffers[2] = {this->_vertex_buffer, this->_index_buffer};
  glDeleteBuffers(2, buffers);
}

void RenderChunk::_UpdateBuffers(std::vector<Vertex> & Verts, std::vector<GLuint> & Indices) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_index_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * Indices.size(), Indices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, this->_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Verts.size(), Verts.data(), GL_DYNAMIC_DRAW);
}

void RenderChunk::Update(srp::RenderState & state) {
  srp::IndexCache index_cache;
  std::vector<GLuint> index_buffer;
  std::vector<Vertex> vertex_data;
  TetrahedronList::iterator it = this->_tetrahedrons.begin();
  std::vector<Vertex>::iterator it2;

  while (it != this->_tetrahedrons.end()) {
    Tetrahedron tet = *(it++);
    unsigned char point_state = tet.GetState(this->_ds, state.GetThreshold());
    if (point_state == 0 || point_state == 0xF) {
      continue;
    }
    tet.Render(this->_ds, state, index_buffer, index_cache, vertex_data);
  }

  this->_UpdateBuffers(vertex_data, index_buffer);

  std::cout << srp::GetMemoryUsage() << std::endl;
}

void RenderChunk::Render(srp::RenderState & State) {
  if (State != this->_last_state) {
    this->Update(State);
    this->_last_state = State;
  }
  //TODO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_index_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vertex_buffer);
}
