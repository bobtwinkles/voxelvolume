#include "Util.hpp"

#include "sys/time.h"
#include "sys/resource.h"

long srp::GetMemoryUsage() {
  struct rusage usage;
  int i = getrusage(RUSAGE_SELF, &usage);
  if (i < 0) {
    return -1;
  }
  return usage.ru_maxrss;
}
