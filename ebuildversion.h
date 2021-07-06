#ifndef EBUILDVERSION_H
#define EBUILDVERSION_H

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

protected:
    bool valid;
    string version;
    double main_version;
    vector<int> main_subversions;
    vector<pair<string, int>> trailing_subversions;

    const static vector<string> ordered_separators;
    const static regex ver_regexp;
};

#endif // EBUILDVERSION_H
