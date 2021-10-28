#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "package.h"

std::string exec(const char* cmd);
size_t pkg_namever_split_pos(const std::string &name_ver);

void skim_spaces_at_the_edges(std::string_view &str);
std::string_view get_pth_enclosed_string_view(const std::string_view &str_view);

std::vector<std::pair<size_t, std::string>> split_string(const std::string &str, const std::vector<std::string> &separators, const size_t first_variable_sep_index = 0);

#endif // UTILS_H
