#ifndef EBUILDVERSION_H
#define EBUILDVERSION_H

#include <unordered_set>
#include <set>
#include <string>
#include <map>
#include <regex>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

class EbuildVersion
{
public:
    EbuildVersion(string ver);
    bool operator < (const EbuildVersion &other);

protected:
    bool valid;
    string version;
    vector<pair<int, long>> version_parsing; // see operator <

    const static vector<string> ordered_separators;
    const static regex ver_regexp;
    const static pair<string,vector<string>> version_single_letter;
    const static int dot_index;
    const static unordered_set<int> smaller_than_nothing_separators;

};

#endif // EBUILDVERSION_H
