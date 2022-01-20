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
        src/ebuild_version.cpp \
        src/main.cpp \
        src/misc_utils.cpp \
        src/named_vector.cpp \
        src/package.cpp \
        src/parser.cpp \
        src/resolver.cpp

HEADERS += \
    src/database.h \
    src/ebuild.h \
    src/ebuild_version.h \
    src/misc_utils.h \
    src/named_vector.h \
    src/package.h \
    src/parser.h \
    src/resolver.h

DISTFILES += \
    README.md \
    notes.md
