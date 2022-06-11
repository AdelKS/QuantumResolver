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
#include <vector>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cctype>

#define LINE_MAX_SIZE 50000

std::string exec(const char* cmd);

std::size_t pkg_namever_split_pos(const std::string_view &name_ver);

void skim_spaces_at_the_edges(std::string_view &str);

std::string_view get_pth_enclosed_string_view(const std::string_view &str_view);

std::vector<std::string> read_file_lines(const std::filesystem::path& file_path);

std::unordered_map<std::string, std::string> read_quoted_vars(const std::filesystem::path& file_path,
                                                                  const std::vector<std::string>& start_with_list);

std::unordered_map<std::string, std::string> read_unquoted_vars(const std::filesystem::path& file_path,
                                                                  const std::vector<std::string>& start_with_list);

std::vector<std::pair<std::string, std::string>> read_quoted_vars(const std::filesystem::path& file_path);

std::vector<std::pair<std::string, std::string>> read_unquoted_vars(const std::filesystem::path& file_path);

std::vector<std::filesystem::path> get_regular_files(const std::filesystem::path &path);

std::string_view get_next_word(std::string_view &words_line);

std::vector<std::pair<std::size_t, std::string_view>> split_string(const std::string_view &str_view,
                                                              const std::vector<std::string> &separators,
                                                              const std::size_t first_variable_sep_index = 0);

std::unordered_set<std::size_t> get_activated_useflags(std::unordered_map<std::size_t, bool> flag_states);

const std::vector<std::filesystem::path> &get_profiles_tree();

std::string to_lower(std::string_view sv);
std::string to_lower(std::string&& sv);

std::string to_upper(std::string_view sv);
std::string to_upper(std::string&& sv);

// Set union

std::unordered_set<std::size_t> operator + (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t> operator + (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t>& operator += (std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

// Set difference

std::unordered_set<std::size_t> operator - (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t> operator - (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

// Interesection

std::unordered_set<std::size_t> operator & (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);


std::unordered_set<std::size_t> operator & (std::unordered_set<std::size_t>&& a,
                                            const std::unordered_set<std::size_t>& b);

std::unordered_set<std::size_t>& operator &= (std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);

// Symmetric difference

std::unordered_set<std::size_t> operator ^ (const std::unordered_set<std::size_t>& a,
                                            const std::unordered_set<std::size_t>& b);


#endif // MISC_UTILS_H
