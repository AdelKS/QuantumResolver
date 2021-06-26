TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        database.cpp \
        ebuild.cpp \
        main.cpp \
        package.cpp \
        resolver.cpp \
        stringindexedvector.cpp \
        utils.cpp

HEADERS += \
    database.h \
    ebuild.h \
    package.h \
    resolver.h \
    stringindexedvector.h \
    utils.h

DISTFILES += \
    notes.md
