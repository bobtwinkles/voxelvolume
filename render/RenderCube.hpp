#ifndef _RENDER_CUBE_H_
#define _RENDER_CUBE_H_

#include "Vec3.hpp"
#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "RenderTypes.hpp"
#include "ogl/VertexBuffer.hpp"

#include <GL/glew.h>
#include <vector>

namespace srp {
  class DataStore;

  void RenderCube(DataStore & DS
                 ,int X
                 ,int Y
                 ,int Z
                 ,unsigned int Threshold
                 ,std::vector<GLuint> & Indicies
                 ,srp::IndexCache & Cache
                 ,std::vector<srp::ogl::Vertex> & Verts);
}

#endif
