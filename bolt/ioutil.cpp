#include "ioutil.h"
#include "os.h"
#include <experimental/filesystem>
#include <numeric_limits>
#include <random>

using fs = std::experimental::filesystem;

std::string next_suffix() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, std::numeric_limits<int>::max());
  int r = dis(gen);
  r = r * 1664525 + 1013904223; // constants from Numerical Recipes
  return std::to_string(r);
}

namespace ioutil {

std::string temp_file(std::string dir, std::string prefix) {
  if (dir == "") {
    dir = os::temp_dir();
  }

  fs::path _dir(dir);

  int nconflict = 0;
  for (int i = 0; i < 10000; i++) {
    fs::path new_temp_dir = _dir / (prefix + next_suffix());
    os.File file =
        os.open_file(path.string(), os.O_RDWR | os.O_CREATE | os.O_EXCL, 0600);
  }
}

std::string temp_dir(std::string dir, std::string prefix) {}
}