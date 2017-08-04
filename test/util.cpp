#include "util.h"
#include "molly/ioutil/tempfile.h"
#include "molly/os/file.h"

namespace os = molly::os;
namespace ioutil = molly::ioutil;

// temp_file returns a temporary file path
std::string temp_file() {
  os::File f = ioutil::temp_file("", "bolt-");
  f.close();
  os::remove(f.name());
  return f.name();
}

DB *must_open_db() {
  std::string name = temp_file();
  DB *db = new DB(name, 0666, nullptr);
  return db;
}