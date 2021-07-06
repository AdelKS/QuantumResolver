#include "utils.h"

vector<pair<string, string>> split_string(const string &str, const vector<string> &separators)
{
    const int N = str.size();
    int n;

    vector<pair<string,string>> split;

    static const string empty;
    const string* matched_sep = &empty;
    int match_start = 0;

    int j = 0;

    for(int i = 0 ; i < N; i++)
    {
        for(const string &sep: separators)
        {
            n = sep.size();

            if(n > N - i)
               continue;

            for(j = 0; j < n && sep[j] == str[i+j]; ++j);

            if(j != n)
                continue;

            split.emplace_back(*matched_sep, str.substr(match_start, i-match_start));
            match_start = i + sep.size();
            i = i + sep.size();
            matched_sep = &sep;
        }
    }
    if(match_start < N)
        split.emplace_back(*matched_sep, str.substr(match_start, N-match_start));

    else if(match_start > N)
        cout << "bug detected with this version " << str << endl;


    if(split.size() > 5)
    {
        cout << "Big version string here! " << endl;
    }

    return split;
}
