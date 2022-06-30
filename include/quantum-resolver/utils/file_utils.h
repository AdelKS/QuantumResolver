#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <vector>

const extern std::vector<std::filesystem::path> flatenned_profiles_tree;

std::unordered_map<std::string, std::string> read_quoted_vars(const std::filesystem::path& file_path,
                                                                  const std::vector<std::string>& start_with_list);

std::unordered_map<std::string, std::string> read_unquoted_vars(const std::filesystem::path& file_path,
                                                                  const std::vector<std::string>& start_with_list);

std::vector<std::pair<std::string, std::string>> read_quoted_vars(const std::filesystem::path& file_path);

std::vector<std::pair<std::string, std::string>> read_unquoted_vars(const std::filesystem::path& file_path);

std::vector<std::filesystem::path> get_regular_files(const std::filesystem::path &path);

std::vector<std::string> read_file_lines(const std::filesystem::path& file_path);

void print_file_contents(const std::filesystem::path& file_path);

