#ifndef PARSEUTILS_H
#define PARSEUTILS_H

#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <filesystem>
#include <deque>
#include <string_view>
#include <unordered_map>

#define LINE_MAX_SIZE 10000

std::string exec(const char* cmd);

size_t pkg_namever_split_pos(const std::string_view &name_ver);

void skim_spaces_at_the_edges(std::string_view &str);

std::string_view get_pth_enclosed_string_view(const std::string_view &str_view);

std::deque<std::string> read_file_lines(const std::filesystem::path file_path,
                                        std::deque<std::string> starts_with = std::deque<std::string>(),
                                        bool omit_comments_and_empty = true,
                                        size_t max_line_size = LINE_MAX_SIZE);

std::deque<std::filesystem::path> get_regular_files(const std::filesystem::path &path);
std::vector<std::pair<size_t, std::string_view>> split_string(const std::string_view &str_view, const std::vector<std::string> &separators, const size_t first_variable_sep_index = 0);


#endif // PARSEUTILS_H
