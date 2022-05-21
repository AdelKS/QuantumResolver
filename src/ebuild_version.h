#ifndef EBUILD_VERSION_H
#define EBUILD_VERSION_H

#include <unordered_set>
#include <string>
#include <regex>
#include <utility>
#include <vector>

struct VersionConstraint;

class EbuildVersion
{
public:
    EbuildVersion(std::string ver = std::string());
    void set_version(std::string ver);

    friend bool operator <  (const EbuildVersion &a, const EbuildVersion &b); // returns true if this < 1.23
    friend bool operator <= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this <= 1.23
    friend bool operator >  (const EbuildVersion &a, const EbuildVersion &b); // returns true if this > 1.23
    friend bool operator >= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this >= 1.23
    friend bool operator == (const EbuildVersion &a, const EbuildVersion &b); // returns true if this = 1.23
    friend bool operator *= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this matches with =1.23*
    friend bool operator ^= (const EbuildVersion &a, const EbuildVersion &b); // returns true if this ~ 1.23

    bool respects_constraint(const VersionConstraint &constraint);

    const std::string &get_version();

protected:
    std::string version;
    std::vector<std::pair<std::size_t, long>> version_parsing; // see operator <

    const static std::vector<std::string> ordered_separators;
    const static std::regex ver_regexp;
    const static std::pair<std::string, std::vector<std::string>> version_single_letter;
    const static std::size_t dot_index;
    const static std::unordered_set<std::size_t> smaller_than_nothing_separators;

};

struct VersionConstraint
{
    enum struct Type {NONE, SLESS, LESS, EQ_REV, EQ_STAR, EQ, GREATER, SGREATER};

    VersionConstraint(): type(Type::NONE), version() {};

    Type type;
    EbuildVersion version;
};



#endif // EBUILD_VERSION_H
