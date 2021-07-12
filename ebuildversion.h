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
    bool operator < (const EbuildVersion &other);

protected:
    bool valid;
    string version;
    vector<pair<int, long>> version_parsing; // see operator <

    const static vector<string> ordered_separators;
    const static regex ver_regexp;
    const static pair<string,vector<string>> version_single_letter;
};

#endif // EBUILDVERSION_H
