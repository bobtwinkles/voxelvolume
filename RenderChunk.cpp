#include "RenderChunk.hpp"
#include "Tetrahedron.hpp"

#include <iostream>

using srp::RenderChunk;
using srp::Tetrahedron;
using srp::Vec3i;

RenderChunk::RenderChunk(srp::DataStore & ds, int X, int Y, int Z) : _ds(ds), _x(X), _y(Y), _z(Z) {
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
}

RenderChunk::~RenderChunk() {
  while (this->_tetrahedrons.size() > 0) {
    Tetrahedron t = this->_tetrahedrons.back();
    this->_tetrahedrons.pop_back();
    delete &t;
  }
}

void RenderChunk::Render(srp::RenderState & state) {
  TetrahedronList::iterator it = this->_tetrahedrons.begin();
  while (it != this->_tetrahedrons.end()) {
    Tetrahedron tet = *(it++);
    unsigned char point_state = tet.GetState(this->_ds, state.GetThreshold());
    if (point_state == 0 || point_state == 0xF) {
      continue;
    }
    tet.Render(this->_ds, state);
  }
}
