### Quantum Resolver

This is an attempt at writing a faster Gentoo dependency resolver with a hope to get it upstreamed and replace that specific part in `emerge`. This project is at its early beginnings.

The current main ideas:
- Map package names, use flags, use expands to unique integers, for example `sys-devel/gcc` -> `13`, `pgo` -> `13` (use flags do not live in the same world as package names), and work only with integers while resolving dependencies.
  - For this purpose, various classes have been implemented (_e.g._ [NamedVector](src/named_vector.h), [MultiKeyMap](src/multikey_map.h) and [Bijection](src/bijection.h) that enable going back and forth between strings and integers.
- Parse the initial files on disk only once and produce a fast intermediate representation that helps speeding up dependency resolution. This applies to _e.g._ to `ebuild` versions (see [ebuild_version.h](src/ebuild_version.h)), dependencies and flags (see [ebuild.h](src/ebuild.h)).


#### Currently exposed feature to the CLI

To properly work, the `quantum` executable needs an up-to-date `md5-cache` in `/var/db/repos/gentoo/metadata/md5-cache/`. One can update it by running the following command as root (it is quite fast)

```shell
egencache --update --repo gentoo
```

Currently, `quantum` offers `status` as a command line argument

```shell
quantum status [atom]
```

where `[atom]` is for example `sys-devel/gcc` or `"=sys-devel/gcc-10.3*"`. It does a mixture between `equery y [atom]` and `emerge -qpvO [atom]`: it outputs the state of the flags for each version that matches `[atom]` if it were to be (re)emerged, while showing any eventual changed use with the same color code as `emerge`.

The idea is to have it display a "matrix" view that gives all the necessary information. This is only a intermediary step that is needed to implement a proper dependency resolver: it needs to know of flag changes, installed packages, flag states... etc.

##### Example
Running the following command
```shell
./quantum status sys-devel/gcc
```

outputs something like this

```shell
##############################
sys-devel/gcc
~~~~~~~~~~~~~~~~~~~~
Shared states
--------------------
USE="(cxx) fortran graphite (multilib) nls nptl openmp pgo (pie) sanitize ssp vtv (-ada) -debug -doc (-fixed-point) -go (-hardened) -jit (-libssp) -objc -objc++ -objc-gc -systemtap -test -vanilla"
~~~~~~~~~~~~~~~~~~~~
Matching versions
--------------------
    8.5.0-r1  USE="pch -mpx"
--------------------
    9.5.0  USE="pch -d -lto"
--------------------
    10.3.0-r2  USE="zstd (-cet) -d -lto (-pch)"
--------------------
    10.3.1_p20211126  USE="zstd (-cet) -d -lto (-pch)"
--------------------
    10.3.1_p20220526  USE="zstd (-cet) -d -lto (-pch)"
--------------------
    10.3.1_p20220602  USE="zstd (-cet) -d -lto (-pch)"
--------------------
    10.4.9999  USE="zstd (-cet) -d -lto (-pch)"
--------------------
    11.2.0  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    11.2.1_p20220115  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
[I] 11.3.0  USE="zstd (-cet) (-custom-cflags) -d -lto* (-pch) -valgrind"
--------------------
    11.3.1_p20220527  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    11.3.1_p20220603  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    11.4.9999  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    12.1.0  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    12.1.1_p20220528  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    12.1.1_p20220528-r1  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    12.1.1_p20220604  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    12.2.9999  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    13.0.0_pre20220529  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    13.0.0_pre20220605  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
--------------------
    13.0.9999  USE="zstd (-cet) (-custom-cflags) -d -lto (-pch) -valgrind"
```

**Notes:**
- The `lto` flag got disabled for demonstration purposes, one can see that it's printed as `-lto*` (with a star at the end, and in green in the terminal) only for the installed version.
- The masked versions are not yet put between parentheses, _e.g._ `(13.0.9999)` instead of the displayed `13.0.9999`.

#### How to (e)build

In its current state, you need `qmake` (I will switch to `meson` when things get more serious)

```shell
qmake CONFIG+="release" QuantumResolver.pro
make
```

For a debug build

```shell
qmake CONFIG+='debug sanitizer sanitize_address' QuantumResolver.pro
make
```

This will create a `quantum` executable in the same folder.

Otherwise, if you have `QtCreator` you can simply open the `.pro` file and setup the project for "Release" and "Debug" builds. Then press the "Play" button.
