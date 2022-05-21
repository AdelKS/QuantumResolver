TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

TARGET = quantum

QMAKE_CFLAGS += -march=native -Wpedantic -Wall -Wconversion
QMAKE_CXXFLAGS += -march=native -Wpedantic -Wall -Wconversion

SOURCES += \
        src/database.cpp \
        src/ebuild.cpp \
        src/ebuild_version.cpp \
        src/main.cpp \
        src/misc_utils.cpp \
        src/named_vector.cpp \
        src/package.cpp \
        src/parser.cpp \
        src/repo.cpp \
        src/resolver.cpp \
        src/useflags.cpp

HEADERS += \
    src/bijection.h \
    src/database.h \
    src/ebuild.h \
    src/ebuild_version.h \
    src/misc_utils.h \
    src/multikeymap.h \
    src/named_vector.h \
    src/package.h \
    src/parser.h \
    src/repo.h \
    src/resolver.h \
    src/useflags.h

DISTFILES += \
    README.md \
    notes.md
