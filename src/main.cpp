#include <iostream>

using namespace std;

#include "database.h"



int main()
{
    Database database;
    try
    {
        database.populate_from_overlay("/var/db/repos/gentoo");
    }
    catch(string str)
    {
        cout << str << endl;
    }

    return 0;
}
