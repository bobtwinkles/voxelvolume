#include "RenderCube.hpp"

#include "DataStore.hpp"
#include <limits.h>

static float inverse_lerp_factor(unsigned int A, unsigned int B, unsigned int Val);
// Point offset specificaton.
// It is [4] not [3] for allignment
char TETRAHEDRON_POINTS[8][4] = {{0, 0, 0, 0}
                                ,{0, 0, 1, 0}
                                ,{0, 1, 0, 0}
                                ,{0, 1, 1, 0}
                                ,{1, 0, 0, 0}
                                ,{1, 0, 1, 0}
                                ,{1, 1, 0, 0}
                                ,{1, 1, 1, 0}};

// Tetrahedrons will be composed as described in this array.
int TETRAHEDRON_MAP[6][4] = {{0, 2, 3, 6}
                            ,{0, 1, 3, 6}
                            ,{1, 0, 4, 6}
                            ,{1, 3, 6, 7}
                            ,{1, 4, 6, 7}
                            ,{1, 4, 5, 7}};

// Lists of edges that form the triangles for a given set of passed values
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

// Map of edge index to point index
int EDGE_MAP[6][2] = {{ 0, 1 }
                     ,{ 0, 2 }
                     ,{ 0, 3 }
                     ,{ 1, 2 }
                     ,{ 1, 3 }
                     ,{ 2, 3 }};

void srp::RenderCube(DataStore & DS
                    ,int X
                    ,int Y
                    ,int Z
                    ,unsigned int Threshold
                    ,std::vector<GLuint> & Indicies
                    ,srp::IndexCache & Cache
                    ,std::vector<srp::ogl::Vertex> & Verts) {
  unsigned int points[8];
  bool points_pass[8];
  Vec3f edges[6];
  // Get local copies of the points to keep them in the cache
  for (int i = 0; i < 8; ++i) {
    points[i] = DS.GetPoint(X + TETRAHEDRON_POINTS[i][0],
                            Y + TETRAHEDRON_POINTS[i][1],
                            Z + TETRAHEDRON_POINTS[i][2]);
    points_pass[i] = points[i] > Threshold;
  }

  for (int tetrahedron = 0; tetrahedron < 6; ++tetrahedron) {
    unsigned char state = 0;
    unsigned char edges_seen = 0; // TODO: there is probably a way to remove this
    for (int i = 0; i < 4; ++i) {
      if (points_pass[TETRAHEDRON_MAP[tetrahedron][i]]) {
        state |= 1 << i;
      }
    }
    if (state == 0 || state == 0xF) {
      continue;
    }

    for (int i = 0; i < 6; ++i) {
      int edge = TRIANGLE_MAP[state][i];
      if (edge == -1) {
        break;
      }

      if (!(edges_seen & (1 << edge))) {
        int a = TETRAHEDRON_MAP[tetrahedron][EDGE_MAP[edge][0]];
        int b = TETRAHEDRON_MAP[tetrahedron][EDGE_MAP[edge][1]];
        float fac = inverse_lerp_factor(points[a], points[b], Threshold);
        float inv_fac = 1 - fac;
        edges[edge] = Vec3f(X + fac * TETRAHEDRON_POINTS[a][0] + inv_fac * TETRAHEDRON_POINTS[b][0]
                           ,Y + fac * TETRAHEDRON_POINTS[a][1] + inv_fac * TETRAHEDRON_POINTS[b][1]
                           ,Z + fac * TETRAHEDRON_POINTS[a][2] + inv_fac * TETRAHEDRON_POINTS[b][2]);
        edges_seen |= 1 << edge;
      }

      srp::IndexCache::iterator it = Cache.find(edges[edge]);
      if (it == Cache.end()) {
        srp::ogl::Vertex v;
        GLuint creation_index = 0;
        v.x = edges[edge].GetX(); v.y = edges[edge].GetY(); v.z = edges[edge].GetZ(); v.w = 0;
        v.r = v.x / 256.f;
        v.g = v.y / 256.f;
        v.b = v.z / 256.f;
        creation_index = Verts.size();
        it = Cache.insert(std::make_pair(edges[edge], creation_index)).first;
        Verts.push_back(v);
      }
      Indicies.push_back(it->second);
    }
  }
}

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

