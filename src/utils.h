#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

vector<pair<int, string>> split_string(const string &str, const vector<string> &separators, const int first_variable_sep_index = 0);

#endif // UTILS_H
