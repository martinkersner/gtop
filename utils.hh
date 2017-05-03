// Martin Kersner, m.kersner@gmail.com
// 2017/04/22 

#ifndef UTILS_HH_
#define UTILS_HH_

#include <vector>
#include <string>
#include <sstream>
#include <sys/stat.h>

typedef std::vector<std::string> vec_str;

vec_str tokenize(const std::string &, const char);

bool file_exists(const std::string &);

#endif // UTILS_HH_
