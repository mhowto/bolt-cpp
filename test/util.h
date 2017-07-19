#include "bolt/db.h"
#include <string>

DB *MustOpenDB() { return open(tempfile(), 0666, nullptr); }

std::string tempfile() {}