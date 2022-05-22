#ifndef DATABASE_H
#define DATABASE_H

#include <memory>

#include "repo.h"
#include "parser.h"
#include "useflags.h"

class Database
{
public:
    Database();

    Parser parser;
    UseFlags useflags;
    Repo repo;
};

#endif // DATABASE_H
