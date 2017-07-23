#ifndef __IOUTIL_H
#define __IOUTIL_H

#include <string>

namespace ioutil {
// temp_file creates a new temporary file in the directory dir with
// a name beginning with prefix, opens the file for reading and writing, and
// returns the resulting file stream.
std::string temp_file(std::string dir, std::string prefix);

std::string temp_dir(std::string dir, std::string prefix);
}

#endif