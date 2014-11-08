#include "Util.hpp"

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <cstring>

#define CWD_BUFLEN 512

static std::string basedir;

long srp::GetMemoryUsage() {
  struct rusage usage;
  int i = getrusage(RUSAGE_SELF, &usage);
  if (i < 0) {
    return -1;
  }
  return usage.ru_maxrss;
}

//// NOTE: ONLY WORK ON *NIX
void srp::InitializeBaseDirectory(const char * Argv0) {
  char cwd[CWD_BUFLEN];
  getcwd(cwd, CWD_BUFLEN);
  strncat(cwd, "/", CWD_BUFLEN);
  strncat(cwd, Argv0, CWD_BUFLEN);

  for (int i = strlen(cwd) - 1; i > 0; --i) {
    if (cwd[i] == '/') {
      cwd[i + 1] = 0;
      break;
    }
  }

  basedir = std::string(cwd);
}

std::string srp::GetBaseDirectory() {
  return basedir;
}
