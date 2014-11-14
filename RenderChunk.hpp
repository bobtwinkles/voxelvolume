#ifndef _RENDER_CHUNK_H_
#define _RENDER_CHUNK_H_

#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "Tetrahedron.hpp"
#include "Util.hpp"
#include "ogl/VertexBuffer.hpp"

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>

namespace srp {
  void RenderChunk(srp::DataStore & Store, int X, int Y, int Z, unsigned int Threshold,
                   srp::IndexCache & Cache, std::vector<GLuint> & Indicies, std::vector<srp::ogl::Vertex> & Verts);
}

#endif
