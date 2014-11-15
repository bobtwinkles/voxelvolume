#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <string>

#define DISALLOW_COPY_AND_ASSIGN(t) \
  t(const t&); \
  void operator=(const t&);

#define ABS(a) ( (a) > 0 ? (a) : (-(a)) )

#define FLOAT_NEAR_EQ(a, b, epsilon) ( ABS((a) - (b)) < epsilon)

// Dereferences a random (invalid) pointer
#define BUG() { std::cerr << __FILE__ << ":" << std::dec << __LINE__ << ": bug =(" << std::endl; *((int*)0x01) = 0; }

namespace srp {
  long GetMemoryUsage();

  void InitializeBaseDirectory(const char * Argv0);
  std::string GetBaseDirectory();
}

#endif
