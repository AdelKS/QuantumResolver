quantum_headers = files(
    'cli/table_print.h',
    'cli/cli_interface.h',
    'cli/format_utils.h',
    'core/ebuild_version.h',
    'core/ebuild.h',
    'core/parser.h',
    'core/useflags.h',
    'core/package.h',
    'core/repo.h',
    'database.h',
    'resolver.h',
    'utils/concepts.h',
    'utils/file_utils.h',
    'utils/misc_utils.h',
    'utils/multikey_map.h',
    'utils/named_vector.h',
    'utils/bijection.h',
    'utils/string_utils.h',
)

if not meson.is_subproject()
    # needed when using `meson install` so headers end up in /usr/include
    install_headers(quantum_headers, subdir : proj_inc_dir, preserve_path : true)
endif
