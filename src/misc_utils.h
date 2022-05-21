#ifndef MISC_UTILS_H
#define MISC_UTILS_H

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
#include <unordered_set>
#include <set>
#include <cctype>

#define LINE_MAX_SIZE 10000

std::string exec(const char* cmd);

std::size_t pkg_namever_split_pos(const std::string_view &name_ver);

void skim_spaces_at_the_edges(std::string_view &str);

std::string_view get_pth_enclosed_string_view(const std::string_view &str_view);

std::deque<std::string> read_file_lines(const std::filesystem::path file_path);

std::deque<std::pair<std::string, std::string>> read_vars(const std::filesystem::path file_path);

std::deque<std::filesystem::path> get_regular_files(const std::filesystem::path &path);

std::string_view get_next_word(std::string_view &words_line);

std::vector<std::pair<std::size_t, std::string_view>> split_string(const std::string_view &str_view,
                                                              const std::vector<std::string> &separators,
                                                              const std::size_t first_variable_sep_index = 0);

std::unordered_set<std::size_t> get_activated_useflags(std::unordered_map<std::size_t, bool> flag_states);

const std::vector<std::filesystem::path> &get_profiles_tree();

std::string to_lower(std::string s);

#endif // MISC_UTILS_H
