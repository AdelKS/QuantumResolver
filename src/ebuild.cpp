#include "ebuild.h"

using namespace std;

Ebuild::Ebuild(string ver): eversion(ver)
{

}

void Ebuild::add_use_flag(int flag, const UseFlagDefaults &defaults)
{
    use_flags[flag] = defaults;
}

bool Ebuild::operator <(const Ebuild &other)
{
    return eversion < other.eversion;
}
