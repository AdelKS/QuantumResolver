TEMPLATE = app
CONFIG += console c++2a
CONFIG -= app_bundle
CONFIG -= qt

TARGET = quantum

QMAKE_CFLAGS += -std=c++20 -march=native -Wpedantic -Wall -Wextra -Wconversion
QMAKE_CXXFLAGS += -std=c++20 -march=native -Wpedantic -Wall -Wextra -Wconversion

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

INCLUDEPATH += include/


DISTFILES += \
    README.md \
    notes.md

SOURCES += \
    src/cli/cli_interface.cpp \
    src/cli/table_print.cpp \
    src/cli/format_utils.cpp \
    src/core/ebuild.cpp \
    src/core/ebuild_version.cpp \
    src/core/package.cpp \
    src/core/parser.cpp \
    src/core/repo.cpp \
    src/core/useflags.cpp \
    src/database.cpp \
    src/quantum.cpp \
    src/resolver.cpp \
    src/utils/file_utils.cpp \
    src/utils/misc_utils.cpp \
    src/utils/string_utils.cpp

HEADERS += \
    include/quantum-resolver/cli/cli_interface.h \
    include/quantum-resolver/cli/table_print.h \
    include/quantum-resolver/cli/format_utils.h \
    include/quantum-resolver/core/ebuild.h \
    include/quantum-resolver/core/ebuild_version.h \
    include/quantum-resolver/core/package.h \
    include/quantum-resolver/core/parser.h \
    include/quantum-resolver/core/repo.h \
    include/quantum-resolver/core/useflags.h \
    include/quantum-resolver/database.h \
    include/quantum-resolver/resolver.h \
    include/quantum-resolver/utils/bijection.h \
    include/quantum-resolver/utils/concepts.h \
    include/quantum-resolver/utils/file_utils.h \
    include/quantum-resolver/utils/misc_utils.h \
    include/quantum-resolver/utils/multikey_map.h \
    include/quantum-resolver/utils/named_vector.h \
    include/quantum-resolver/utils/string_utils.h
