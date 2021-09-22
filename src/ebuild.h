#ifndef EBUILD_H
#define EBUILD_H

#include <string>

#include "ebuildversion.h"
#include "indexedvector.h"

class UseFlagDefaults
{

public:
    UseFlagDefaults(bool has_default = false, bool default_positive = true){
        this->has_default = has_default;
        this->default_positive = default_positive;
    }

    bool has_default, default_positive;
};

class Ebuild
{

public:
    Ebuild(std::string ver);

    bool operator <(const Ebuild &other);
    void add_use_flag(int flag, const UseFlagDefaults &defaults = UseFlagDefaults());


protected:
    std::unordered_map<int, UseFlagDefaults> use_flags;
    EbuildVersion eversion;
};



#endif // EBUILD_H
