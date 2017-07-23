#include "os.h"
#include "syscall.h"
#include <cstring>
#include <experimental/filesystem>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using fs = std::experimental::filesystem;

namespace os {
std::string temp_dir() {
  std::string dir = syscall.getenv("TMPDIR");
  if (dir == "") {
    fs::path path = fs::temp_directory_path();
    return path.string();
  }
  return dir;
}

file_info stat(std::string name) { syscall.stat(name); }
}