#include <iostream>
#include <chrono>
using namespace std::chrono;

using namespace std;

#include "database.h"

int main(int argc, char *argv[])
{
    cout << argc << " " << argv << endl;
    try
    {
        Database database;
    }
    catch(runtime_error &err)
    {
        cout << err.what() << endl;
    }

    return 0;
}
