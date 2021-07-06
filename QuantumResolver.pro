TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        database.cpp \
        ebuild.cpp \
        ebuildversion.cpp \
        indexedvector.cpp \
        main.cpp \
        package.cpp \
        resolver.cpp \
        utils.cpp

HEADERS += \
    database.h \
    ebuild.h \
    ebuildversion.h \
    indexedvector.h \
    package.h \
    resolver.h \
    utils.h

DISTFILES += \
    notes.md
