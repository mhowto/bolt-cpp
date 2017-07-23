#include "syscall.h"
#include <cstdlib>

namespace syscall {
std::string getenv(std::string key) {
  const char *env_p = std::get_env(key.c_str());
  if (!env_p) {
    return "";
  }
  std::string env(env_p);
  delete[] env_p;
  return env;
}
}