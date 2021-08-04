#ifndef EBUILDVERSION_H
#define EBUILDVERSION_H

#include <unordered_set>
#include <string>
#include <regex>
#include <utility>
#include <vector>

class EbuildVersion
{
public:
    EbuildVersion(std::string ver);
    bool operator < (const EbuildVersion &other);

protected:
    bool valid;
    std::string version;
    std::vector<std::pair<int, long>> version_parsing; // see operator <

    const static std::vector<std::string> ordered_separators;
    const static std::regex ver_regexp;
    const static std::pair<std::string, std::vector<std::string>> version_single_letter;
    const static int dot_index;
    const static std::unordered_set<int> smaller_than_nothing_separators;

};

#endif // EBUILDVERSION_H
