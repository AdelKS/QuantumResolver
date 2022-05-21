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

    Repo repo;
    Parser parser;
    UseFlags useflags;
};

#endif // DATABASE_H
