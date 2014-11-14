#include "RenderChunk.hpp"
#include "RenderTypes.hpp"
#include "Tetrahedron.hpp"
#include "ogl/OGLUtil.hpp"

#include <iostream>

using srp::Tetrahedron;
using srp::Vec3i;
using srp::ogl::Vertex;

void srp::RenderChunk(srp::DataStore & ds, int X, int Y, int Z, unsigned int Threshold
                     ,srp::IndexCache & Cache, std::vector<GLuint> & Indicies, std::vector<Vertex> & Verts) {
  Tetrahedron cube[6];

  for (int x = X * RENDER_CHUNK_SIZE; x < (X + 1) * RENDER_CHUNK_SIZE; ++x) {
    for (int y = Y * RENDER_CHUNK_SIZE; y < (Y + 1) * RENDER_CHUNK_SIZE; ++y) {
      for (int z = Z * RENDER_CHUNK_SIZE; z < (Z + 1) * RENDER_CHUNK_SIZE; ++z) {
        srp::BuildForCube(x, y, z, cube);
        for (int i = 0; i < 6; ++i) {
          unsigned char point_state = cube[i].GetState(ds, Threshold);
          if (point_state == 0 || point_state == 0xF) {
            continue;
          }
          cube[i].Render(ds, Threshold, Indicies, Cache, Verts);
        }
      }
    }
  }
}
