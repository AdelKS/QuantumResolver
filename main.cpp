#include <iostream>

using namespace std;

#include "database.h"

int main()
{
    Database database;
    database.populate_from_overlay("/var/db/repos/gentoo");
    return 0;
}
