#include "utils.h"

#include <iostream>
#include <cstring>

vector<pair<int, string>> split_string(const string &str, const vector<string> &separators)
{
    /* splits string str into vector of pairs
     * with str="5.12.3_pre324_p32-r23", separators=[".", "_pre", "-r"]
     * the returned vector would be [("", "5"), (".", "12"), (".", "3"), ("_pre", "324"), ("_p", "32"), ("-r", "23")]
     * */

    const int N = str.size();
    int n;

    vector<pair<int, string>> split;

    int curr_sep_index = 0;
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


    if(split.size() > 5)
    {
        cout << "Big version string here! " << endl;
    }

    return split;
}
