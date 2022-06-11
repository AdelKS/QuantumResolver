TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

TARGET = quantum

QMAKE_CFLAGS += -std=c++20 -march=native -Wpedantic -Wall -Wconversion
QMAKE_CXXFLAGS += -std=c++20 -march=native -Wpedantic -Wall -Wconversion

SOURCES += \
        src/cli_interface.cpp \
        src/database.cpp \
        src/ebuild.cpp \
        src/ebuild_version.cpp \
        src/main.cpp \
        src/misc_utils.cpp \
        src/package.cpp \
        src/parser.cpp \
        src/repo.cpp \
        src/resolver.cpp \
        src/useflags.cpp

HEADERS += \
    src/bijection.h \
    src/cli_interface.h \
    src/concepts.h \
    src/database.h \
    src/ebuild.h \
    src/ebuild_version.h \
    src/format_utils.h \
    src/misc_utils.h \
    src/multikey_map.h \
    src/named_vector.h \
    src/package.h \
    src/parser.h \
    src/repo.h \
    src/resolver.h \
    src/useflags.h

DISTFILES += \
    README.md \
    notes.md
