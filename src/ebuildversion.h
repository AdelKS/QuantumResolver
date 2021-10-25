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
    EbuildVersion(std::string ver = std::string());
    void set_version_str(std::string ver);

    friend bool operator <  (const EbuildVersion &a, const EbuildVersion &b); // returns true if this < 1.23
    friend bool operator <= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this <= 1.23
    friend bool operator >  (const EbuildVersion &a, const EbuildVersion &b); // returns true if this > 1.23
    friend bool operator >= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this >= 1.23
    friend bool operator == (const EbuildVersion &a, const EbuildVersion &b); // returns true if this = 1.23
    friend bool operator *= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this matches with =1.23*
    friend bool operator ^= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this ~ 1.23

protected:
    std::string version;
    std::vector<std::pair<size_t, long>> version_parsing; // see operator <

    const static std::vector<std::string> ordered_separators;
    const static std::regex ver_regexp;
    const static std::pair<std::string, std::vector<std::string>> version_single_letter;
    const static size_t dot_index;
    const static std::unordered_set<size_t> smaller_than_nothing_separators;

};

struct VersionConstraint
{
    enum struct Type {NONE, SLESS, LESS, EQ_REV, EQ_STAR, EQ, GREATER, SGREATER};

    Type type;
    EbuildVersion version;
};

bool respects_constraint(const EbuildVersion &ver, const VersionConstraint &constraint);



#endif // EBUILDVERSION_H
