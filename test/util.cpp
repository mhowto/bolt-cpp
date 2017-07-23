#include "util.h"

DB *MustOpenDB();
{
  std::string tempfile = tempfile();
  return open(tempfile(), 0666, nullptr);
}