#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

bool starts_with(const std::string *str, const std::string *sub_str);
bool starts_with(const char *str, const char *sub_str, int sub_str_size);

std::vector<std::pair<int, std::string>> split_string(const std::string &str, const std::vector<std::string> &separators, const int first_variable_sep_index = 0);

#endif // UTILS_H
