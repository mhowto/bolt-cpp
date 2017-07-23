#ifndef __UTIL_H
#define __UTIL_H

#include "bolt/db.h"
#include "molly/ioutil/ioutil.h"
#include "molly/os/file.h"
#include <string>

namespace os = molly::os;
namespace ioutil = molly::ioutil;

DB *MustOpenDB() {
  ioutil std::string tempfile = tempfile();
  return open(tempfile(), 0666, nullptr);
}

#endif