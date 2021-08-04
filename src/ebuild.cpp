#include "ebuild.h"

using namespace std;

Ebuild::Ebuild(string ver): eversion(ver)
{

}

bool Ebuild::operator <(const Ebuild &other)
{
    return eversion < other.eversion;
}
