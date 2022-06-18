#pragma once

#include "database.h"
#include "format_utils.h"

class CommandLineInterface
{
public:
    CommandLineInterface(std::vector<std::string> input);

    void print_pkg_status(const std::string &package_constraint_str);

protected:
    Database db;
};
