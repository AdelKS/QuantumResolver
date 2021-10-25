TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

TARGET = quantumresolve

QMAKE_CFLAGS += -march=native
QMAKE_CXXFLAGS += -march=native

SOURCES += \
        src/database.cpp \
        src/ebuild.cpp \
        src/ebuildversion.cpp \
        src/indexedvector.cpp \
        src/main.cpp \
        src/package.cpp \
        src/resolver.cpp \
        src/utils.cpp

HEADERS += \
    src/database.h \
    src/ebuild.h \
    src/ebuildversion.h \
    src/indexedvector.h \
    src/package.h \
    src/resolver.h \
    src/utils.h

DISTFILES += \
    README.md \
    notes.md
