#include "Tetrahedron.hpp"

#include "RenderConfig.hpp"
#include "Vec3f.hpp"
#include "Util.hpp"

#include <GL/glew.h>
#include <GL/gl.h>

#include <iostream>

using srp::DataStore;
using srp::RenderState;
using srp::Tetrahedron;
using srp::Vec3i;
using srp::Vec3f;

Tetrahedron::Tetrahedron(Vec3i P1, Vec3i P2, Vec3i P3, Vec3i P4) {
  _corners[0] = P1;
  _corners[1] = P2;
  _corners[2] = P3;
  _corners[3] = P4;
}

int EDGE_MAP[6][2] = {{ 0, 1 }
                     ,{ 0, 2 }
                     ,{ 0, 3 }
                     ,{ 1, 2 }
                     ,{ 1, 3 }
                     ,{ 2, 3 }};

float EDGE_COLORS[6][3] = {{1.0f, 0.0f, 0.0f}
                          ,{0.0f, 1.0f, 0.0f}
                          ,{0.0f, 0.0f, 1.0f}
                          ,{1.0f, 1.0f, 0.0f}
                          ,{1.0f, 0.0f, 1.0f}
                          ,{1.0f, 1.0f, 1.0f}};

/**
 * corner 0 - corner 1 = 0
 * corner 0 - corner 2 = 1
 * corner 0 - corner 3 = 2
 * corner 1 - corner 2 = 3
 * corner 1 - corner 3 = 4
 * corner 2 - corner 3 = 5
 **/

int TRIANGLE_MAP[64][6] = {{-1, -1, -1, -1, -1, -1}  /*                              */
                          ,{ 0,  1,  2, -1, -1, -1}  /* corner 0                     */
                          ,{ 0,  3,  4, -1, -1, -1}  /* corner 1                     */
                          ,{ 1,  2,  3,  2,  3,  4}  /* corner 0, corner 1           */
                          ,{ 1,  3,  5, -1, -1, -1}  /* corner 2                     */
                          ,{ 0,  2,  3,  2,  3,  5}  /* corner 0, corner 2           */
                          ,{ 0,  1,  5,  0,  2,  5}  /* corner 1, corner 2           */
                          ,{ 2,  4,  5, -1, -1, -1}  /* corner 0, corner 1, corner 2 */
                          ,{ 2,  4,  5, -1, -1, -1}  /* corner 3                     */
                          ,{ 0,  1,  5,  0,  4,  5}  /* corner 0, corner 3           */
                          ,{ 5,  2,  3,  0,  2,  3}  /* corner 1, corner 3           */
                          ,{ 1,  3,  5, -1, -1, -1}  /* corner 0, corner 1, corner 3 */
                          ,{ 4,  2,  3,  3,  1,  2}  /* corner 2, corner 3           */
                          ,{ 0,  3,  4, -1, -1, -1}  /* corner 0, corner 2, corner 3 */
                          ,{ 0,  1,  2, -1, -1, -1}  /* corner 1, corner 2, corner 3 */
                          ,{-1, -1, -1, -1, -1, -1}};/* all the things               */

float TRI_COLORS[64][3] = {{0.0f, 0.0f, 0.0f} /*                              */
                          ,{1.0f, 1.0f, 1.0f} /* corner 0                     */
                          ,{1.0f, 0.0f, 0.0f} /* corner 1                     */
                          ,{0.0f, 0.0f, 0.0f} /* corner 0, corner 1           */
                          ,{0.0f, 0.5f, 0.0f} /* corner 2                     */
                          ,{0.0f, 1.0f, 0.0f} /* corner 0, corner 2           */
                          ,{0.0f, 0.0f, 0.5f} /* corner 1, corner 2           */
                          ,{0.0f, 0.0f, 1.0f} /* corner 0, corner 1, corner 2 */
                          ,{0.5f, 0.5f, 0.5f} /* corner 3                     */
                          ,{1.0f, 1.0f, 1.0f} /* corner 0, corner 3           */
                          ,{1.0f, 1.0f, 0.0f} /* corner 1, corner 3           */
                          ,{0.0f, 1.0f, 1.0f} /* corner 0, corner 1, corner 3 */
                          ,{1.0f, 0.0f, 1.0f} /* corner 2, corner 3           */
                          ,{0.2f, 0.2f, 0.2f} /* corner 0, corner 2, corner 3 */
                          ,{0.2f, 0.2f, 0.2f} /* corner 1, corner 2, corner 3 */
                          ,{1.0f, 1.0f, 1.0f}}/* all the things               */;

static float inverse_lerp_factor(unsigned int A, unsigned int B, unsigned int Val) {
  float tr =  ((float)Val - (float)B) / ((float)A - (float)B);
  if (tr < 0) {
    return 0;
  }
  if (tr > 1) {
    return 1;
  }
  return tr;
}

void Tetrahedron::Render(DataStore & DS,
                         RenderState & State,
                         std::vector<GLuint> & Indicies,
                         srp::IndexCache & Cache,
                         std::vector<srp::Vertex> VertexData) {
  Vec3f edges[6];
  bool edges_seen[6] = {false, false, false, false, false, false};
  unsigned char tri = GetState(DS, State.GetThreshold());

  glColor3f(_corners[0].GetX()/(float)RENDER_CHUNK_SIZE,
            _corners[0].GetY()/(float)RENDER_CHUNK_SIZE,
            _corners[0].GetZ()/(float)RENDER_CHUNK_SIZE);
  //glColor3f(TRI_COLORS[tri][0], TRI_COLORS[tri][1], TRI_COLORS[tri][2]);
  //glColor3f(EDGE_COLORS[_index][0], EDGE_COLORS[_index][1], EDGE_COLORS[_index][2]);
  for (int i = 0; i < 6; ++i) {
    int edge = TRIANGLE_MAP[tri][i];
    if (edge == -1) {
      break;
    }
    if (!edges_seen[edge]) {
      Vec3i a = _corners[EDGE_MAP[edge][0]];
      Vec3i b = _corners[EDGE_MAP[edge][1]];
      float fac = inverse_lerp_factor(DS.GetPoint(a)
                                     ,DS.GetPoint(b)
                                     ,State.GetThreshold());
      edges[edge] = Vec3f(fac * a.GetX() + (1 - fac) * b.GetX()
                         ,fac * a.GetY() + (1 - fac) * b.GetY()
                         ,fac * a.GetZ() + (1 - fac) * b.GetZ());
      edges_seen[edge] = true;

      if (Cache.find(edges[edge]) == Cache.end()) {
        Vertex v;
        GLuint creation_index = 0;
        v.x = edges[edge].GetX(); v.y = edges[edge].GetY(); v.z = edges[edge].GetZ();
        v.nx = v.ny = v.nz = 0;
        v.r = v.x / 256.f;
        v.g = v.y / 256.f;
        v.b = v.z / 256.f;
        v.u = 0;
        v.v = 0;
        creation_index = VertexData.size();
        Cache.insert(std::make_pair(edges[edge], creation_index));
        VertexData.push_back(v);
      }
    }
//    Vec3f point = edges[edge];
//    glColor3f(EDGE_COLORS[edge][0], EDGE_COLORS[edge][1], EDGE_COLORS[edge][2]);
//    glColor3f(point.GetX() / 256.f
//             ,point.GetY() / 256.f
//             ,point.GetZ() / 256.f);
//    glVertex3f(point.GetX(), point.GetY(), point.GetZ());
  }
}

unsigned char Tetrahedron::GetState(srp::DataStore & DS, unsigned int Threshold) {
  unsigned char tr = 0;

  for (int i = 0; i < 4; ++i) {
    if (DS.GetPoint(_corners[i]) > Threshold) {
      tr |= 1 << i;
    }
  }

  return tr;
}

Tetrahedron &Tetrahedron::operator= (Tetrahedron & Other) {
  if (this == &Other) {
    return *this;
  }

  for (unsigned i = 0; i < 4; ++i) {
    this->_corners[i] = Other._corners[i];
  }

  return *this;
}

Tetrahedron ** srp::BuildForCube(int X, int Y, int Z, Tetrahedron ** Base) {
  Base[0] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 0), Vec3i(X + 0, Y + 1, Z + 0), Vec3i(X + 0, Y + 1, Z + 1), Vec3i(X + 1, Y + 1, Z + 0));
  Base[1] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 0), Vec3i(X + 0, Y + 0, Z + 1), Vec3i(X + 0, Y + 1, Z + 1), Vec3i(X + 1, Y + 1, Z + 0));
  Base[2] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 1), Vec3i(X + 0, Y + 0, Z + 0), Vec3i(X + 1, Y + 0, Z + 0), Vec3i(X + 1, Y + 1, Z + 0));
  Base[3] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 1), Vec3i(X + 0, Y + 1, Z + 1), Vec3i(X + 1, Y + 1, Z + 0), Vec3i(X + 1, Y + 1, Z + 1));
  Base[4] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 1), Vec3i(X + 1, Y + 0, Z + 0), Vec3i(X + 1, Y + 1, Z + 0), Vec3i(X + 1, Y + 1, Z + 1));
  Base[5] = new Tetrahedron(Vec3i(X + 0, Y + 0, Z + 1), Vec3i(X + 1, Y + 0, Z + 0), Vec3i(X + 1, Y + 0, Z + 1), Vec3i(X + 1, Y + 1, Z + 1));
  return Base + 6;
}
