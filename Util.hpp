#ifndef _UTIL_H_
#define _UTIL_H_

#define DISALLOW_COPY_AND_ASSIGN(t) \
  t(const t&); \
  void operator=(const t&);

#define ABS(a) ( (a) > 0 ? (a) : (-(a)) )

#define FLOAT_NEAR_EQ(a, b, epsilon) ( ABS((a) - (b)) < epsilon)

// Dereferences a random (invalid) pointer
#define BUG() *((int*)0x01) = 0

#define GLERR() { \
  int the_gl_error_pls = glGetError(); \
  if (the_gl_error_pls != 0) { \
    std::cerr << __FILE__ << ":" << std::dec << __LINE__ << ": OpenGL bug! " << gluErrorString(the_gl_error_pls) << std::endl; \
    exit(-1); \
  } \
  }

namespace srp {
  long GetMemoryUsage();
}

#endif
