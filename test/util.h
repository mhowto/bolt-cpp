#include "bolt/db.h"
#include <experimental/filesystem>
#include <string>

namespace fs = std::experimental::filesystem;

// tempfile returns a temporary file path.
std::string tempfile() {
  fs::path path = fs::temp_directory_path();
  return path;
}

DB *MustOpenDB() {
  path.std::string _tempfile = tempfile();
  return open(tempfile(), 0666, nullptr);
}