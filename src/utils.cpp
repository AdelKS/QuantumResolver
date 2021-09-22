#include "utils.h"

#include <iostream>
#include <cstring>

using namespace std;

bool starts_with(const string *str, const string *sub_str)
{
    if(sub_str->size() > str->size())
        return false;
    else return strncmp(str->c_str(), sub_str->c_str(), str->size()) == 0;
}

bool starts_with(const char *str, const char *sub_str, int sub_str_size)
{
    // Warning! sub_str needs to be smaller
    return strncmp(str, sub_str, sub_str_size) == 0;
}

vector<pair<int, string>> split_string(const string &str, const vector<string> &separators, const int first_variable_sep_index)
{
    /* splits string str into vector of pairs,
     * first_variable_sep_index is the index of the very first string that is split, who does not necessarily have a separator before it
     * Example:
     *  str="5.12.3_pre324_p32-r23", separators=[".", "_p", "_pre", "-r"], first_variable_sep_index=666
     *  the returned vector would be [(666, "5"), (0, "12"), (0, "3"), (2, "324"), (1, "32"), (3, "23")]
     *  where the first index of each couple is the index of the matched separator 0 -> ".", 1 -> "_p", 2 -> "_pre", 3 -> "-r
     * */

    const int N = str.size();
    int n;

    vector<pair<int, string>> split;

    int curr_sep_index = first_variable_sep_index;
    int new_sep_index = 0;
    int match_start = 0;
    bool match;

    for(int i = 0 ; i < N; i++)
    {
        match = false;

        for(int sep_index = 0 ; sep_index < int(separators.size()) ; ++sep_index)
        {
            const string &sep = separators[sep_index];
            n = sep.size();

            if(n > N - i)
               continue;

            // Use strncmp to leverage SIMD instructions
            if(strncmp(str.c_str()+i, sep.c_str(), n) != 0)
                continue;

            if(not match)
            {
                match = not match;
                new_sep_index = sep_index;
            }
            else if(sep.size() > separators[new_sep_index].size())
                new_sep_index = sep_index;
        }

        if(match)
        {
            const string &new_sep = separators[new_sep_index];
            split.emplace_back(curr_sep_index, str.substr(match_start, i-match_start));
            match_start = i + new_sep.size();
            i += new_sep.size() - 1; // the for loop will increment that -1
            curr_sep_index = new_sep_index;
        }
    }
    if(match_start < N)
        split.emplace_back(curr_sep_index, str.substr(match_start, N-match_start));

    else if(match_start > N)
        throw "string splitting failed, string=" + str;

    return split;
}
