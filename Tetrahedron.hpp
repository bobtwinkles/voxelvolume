#ifndef _TETRAHEDERON_H_
#define _TETRAHEDERON_H_

#include "Vec3.hpp"
#include "DataStore.hpp"
#include "RenderConfig.hpp"
#include "RenderTypes.hpp"

#include <GL/glew.h>
#include <vector>

namespace srp {

  class Tetrahedron {
    private:
      Vec3i _corners[4];

      Vec3f _edges[6];

    public:
      Tetrahedron(Vec3i P1, Vec3i P2, Vec3i P3, Vec3i P4);

      void Render(srp::DataStore & ds, RenderState & State, std::vector<GLuint> &, srp::IndexCache &, std::vector<srp::Vertex>);

      /// Returns the set of points which are above the threshold
      ///
      /// @param DS the datastore to reference
      /// @param Threshold thet maximum threshold
      unsigned char GetState(srp::DataStore & ds, unsigned int Threshold);

      Tetrahedron & operator=(Tetrahedron & Other);
  };

  /// Build a cube using 6 tetrahedrons.
  ///
  /// @param X The X coordinate of the cube
  /// @param Y The Y coordinate of the cube
  /// @param Z The Z coordinate of the cube
  /// @param Base an array of Tetrahedrons with at least 6 elements remaining.
  Tetrahedron ** BuildForCube(int X, int Y, int Z, Tetrahedron ** Base);
}

#endif
