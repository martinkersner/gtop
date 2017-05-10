// Martin Kersner, m.kersner@gmail.com
// 2017/04/22 

#include "utils.hh"

vec_str tokenize(const std::string & str, const char delim) {
  vec_str tokens;
  std::stringstream ss(str);

  while (ss.good()) {
    std::string substr;
    getline(ss, substr, delim);
    tokens.push_back(substr);
  }

  return tokens;
}

bool file_exists(const std::string & name) {
  struct stat buffer;
  std::string full_name;
  wordexp_t expanded_name;

  wordexp(name.c_str(), &expanded_name, 0);
  full_name = expanded_name.we_wordv[0];
  wordfree(&expanded_name);

  return (stat(full_name.c_str(), &buffer) == 0);
}
