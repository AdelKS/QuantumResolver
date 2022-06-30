project('Quantum Resolver', 'cpp',
    meson_version: '>=0.63',
    version : '0.0.1',
    default_options : [
        'cpp_std=c++20',
        'buildtype=release',
        'optimization=3',
        'default_library=shared',
        'c_args=-pipe',
        'cpp_args=-pipe'
    ]
)

fmt_dep = dependency('fmt')

proj_inc_dir = 'quantum-resolver'

quantum_resolver_inc = include_directories('include')

subdir('include/quantum-resolver')
subdir('src')
