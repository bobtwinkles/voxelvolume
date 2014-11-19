#ifndef _GEOMETRY_GENERATOR_H_
#define _GEOMETRY_GENERATOR_H_

#include "DataStore.hpp"
#include "VertexBuffer.hpp"

namespace srp {
  int StartGeometryGenerator(srp::DataStore * DS);
  /// Returns true if reading the geometry worked, false otherwise
  bool ReadGeometry(std::vector<GLuint> & Indicies, std::vector<srp::ogl::Vertex> & Verts, unsigned int Threshold);

  void RequestChunk(int X, int Y, int Z, int Threshould);
}

#endif //_GEOMETRY_GENERATOR_H_
