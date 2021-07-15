#include "ebuild.h"

Ebuild::Ebuild(string ver): eversion(ver)
{

}

bool Ebuild::operator <(const Ebuild &other)
{
    return eversion < other.eversion;
}
