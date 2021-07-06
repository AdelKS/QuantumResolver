#ifndef EBUILD_H
#define EBUILD_H

#include "ebuildversion.h"
#include "indexedvector.h"

using namespace std;

class Ebuild
{

public:
    Ebuild(string ver);

    bool operator <(const Ebuild &other);

protected:
    EbuildVersion eversion;
};



#endif // EBUILD_H
