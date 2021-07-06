#include "ebuild.h"

Ebuild::Ebuild(string ver): eversion(ver)
{

}

bool Ebuild::operator <(const Ebuild &other)
{
    return false;
}
