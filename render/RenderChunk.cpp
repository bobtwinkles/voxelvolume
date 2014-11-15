#include "RenderChunk.hpp"
#include "RenderTypes.hpp"
#include "RenderCube.hpp"
#include "ogl/OGLUtil.hpp"

#include <iostream>

using srp::ogl::Vertex;

void srp::RenderChunk(srp::DataStore & ds, int X, int Y, int Z, unsigned int Threshold
                     ,srp::IndexCache & Cache, std::vector<GLuint> & Indicies, std::vector<Vertex> & Verts) {
  for (int z = Z * RENDER_CHUNK_SIZE; z < (Z + 1) * RENDER_CHUNK_SIZE; ++z) {
    for (int x = X * RENDER_CHUNK_SIZE; x < (X + 1) * RENDER_CHUNK_SIZE; ++x) {
      for (int y = Y * RENDER_CHUNK_SIZE; y < (Y + 1) * RENDER_CHUNK_SIZE; ++y) {
        srp::RenderCube(ds, x, y, z, Threshold, Indicies, Cache, Verts);
      }
    }
  }
}
