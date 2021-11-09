TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

TARGET = quantum

Release:QMAKE_CFLAGS += -march=native -O3
Release:QMAKE_CXXFLAGS += -march=native -O3

Debug:QMAKE_CFLAGS += -march=native
Debug:QMAKE_CXXFLAGS += -march=native

SOURCES += \
        src/database.cpp \
        src/ebuild.cpp \
        src/ebuildversion.cpp \
        src/main.cpp \
        src/namedvector.cpp \
        src/package.cpp \
        src/parser.cpp \
        src/parseutils.cpp \
        src/resolver.cpp

HEADERS += \
    src/database.h \
    src/ebuild.h \
    src/ebuildversion.h \
    src/namedvector.h \
    src/package.h \
    src/parser.h \
    src/parseutils.h \
    src/resolver.h

DISTFILES += \
    README.md \
    notes.md
