#ifndef _OGL_UTIL_H_
#define _OGL_UTIL_H_

#include <iostream>

#include "Util.hpp"

#define GLERR() { \
    int the_gl_error_pls = glGetError(); \
    if (the_gl_error_pls != 0) { \
      std::cerr << __FILE__ << ":" << std::dec << __LINE__ << ": OpenGL bug! " << gluErrorString(the_gl_error_pls) << std::endl; \
      BUG(); /* crash in such a way that we get a trace */ \
    } \
  }

#define OFFSET(a) ((void*)((char*)NULL + ((a)*sizeof(float))))

namespace srp {
}

#endif // _OGL_UTIL_H_
