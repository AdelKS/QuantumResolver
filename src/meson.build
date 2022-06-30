quantum_sources = files(
    'cli/cli_interface.cpp',
    'cli/table_print.cpp',
    'core/ebuild.cpp',
    'core/ebuild_version.cpp',
    'core/package.cpp',
    'core/parser.cpp',
    'core/repo.cpp',
    'core/useflags.cpp',
    'database.cpp',
    'resolver.cpp',
    'utils/file_utils.cpp',
    'utils/format_utils.cpp',
    'utils/misc_utils.cpp',
    'utils/string_utils.cpp',
)

quantum_resolver_lib = library('quantum-resolver',
    quantum_sources,
    include_directories : quantum_resolver_inc,
    dependencies : [fmt_dep],
    install : not meson.is_subproject()
)

test_rhc_exe = executable('quantum',
    'quantum.cpp',
    include_directories : quantum_resolver_inc,
    link_with : quantum_resolver_lib,
    install : true)
