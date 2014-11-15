#ifndef _RENDER_TYPES_H_
#define _RENDER_TYPES_H_

#include "Vec3f.hpp"

#include <unordered_map>
#include <GL/glew.h>

namespace srp {
  typedef std::unordered_map<srp::Vec3f, GLuint> IndexCache;
}

#endif
