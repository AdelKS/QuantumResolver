#ifndef DATABASE_H
#define DATABASE_H

#include <memory>

#include "quantum-resolver/core/repo.h"
#include "quantum-resolver/core/parser.h"
#include "quantum-resolver/core/useflags.h"

class Database
{
public:
    Database();

    Parser parser;
    UseFlags useflags;
    Repo repo;
};

#endif // DATABASE_H
