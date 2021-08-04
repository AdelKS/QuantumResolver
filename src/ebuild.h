#ifndef EBUILD_H
#define EBUILD_H

#include <string>

#include "ebuildversion.h"
#include "indexedvector.h"


class Ebuild
{

public:
    Ebuild(std::string ver);

    bool operator <(const Ebuild &other);

protected:
    EbuildVersion eversion;
};



#endif // EBUILD_H
