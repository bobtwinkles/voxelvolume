#ifndef _UTIL_H_
#define _UTIL_H_

#define DISALLOW_COPY_AND_ASSIGN(t) \
  t(const t&); \
  void operator=(const t&);

#define ABS(a) ( (a) > 0 ? (a) : (-(a)) )

#define FLOAT_NEAR_EQ(a, b, epsilon) ( ABS((a) - (b)) < epsilon)

// Dereferences a random (invalid) pointer
#define BUG() *((int*)0x01) = 0

#endif
