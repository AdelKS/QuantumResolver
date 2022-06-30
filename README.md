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

where `[atom]` is for example `sys-devel/gcc` or `"=sys-devel/gcc-10.3*"`. It does a mixture between `equery y [atom]` and `emerge -qpvO [atom]`:
- Shows the keywording of the matched ebuilds (with the same code as `equery`) on the arch you are running, and if they are accepted (with a 'y' or 'n')
- Show the slot of each version
- Outputs the state of the flags for each version that matches `[atom]` if it were to be (re)emerged.
- Shows any eventual changed use with the same color code as `emerge`.

The idea is to have it display a table view (will be implemented) that gives all the necessary information. This is only a intermediary step that is needed to implement a proper dependency resolver: it needs to know of flag changes, installed packages, flag states... etc.

##### Example
Running the following command
```shell
./quantum status sys-devel/gcc
```

outputs something like this (please give a try to the code to see it with color coding)

```shell
##############################
sys-devel/gcc    @system    @selected-packages
~~~~~~~~~~~~~

Shared flag states
~~~~~~~~~~~~~~~~~~
USE="(cxx) fortran graphite (multilib) nls nptl openmp pgo (pie) sanitize ssp vtv (-ada) -debug
      -doc (-fixed-point) -go (-hardened) -jit (-libssp) -objc -objc++ -objc-gc -systemtap -test -vanilla"

Matching versions
~~~~~~~~~~~~~~~~~~

                         | amd64 | SLOT  | non shared flag states                   |
-------------------------+-------+-------+------------------------------------------|
     8.5.0-r1            | + y   | 8.5.0 | USE += "pch -mpx"                        |
-------------------------+-------+-------+------------------------------------------|
     9.5.0               | + y   | 9.5.0 | USE += "lto pch -d"                      |
-------------------------+-------+-------+------------------------------------------|
     10.3.0-r2           | + y   | 10    | USE += "lto pch zstd (-cet) -d"          |
     10.3.1_p20211126    | + y   |       | USE += "lto pch zstd (-cet) -d"          |
     10.3.1_p20220623    | o n   |       | USE += "lto pch zstd (-cet) -d"          |
     10.4.9999           | o n   |       | USE += "lto zstd (-cet) -d (-pch)"       |
-------------------------+-------+-------+------------------------------------------|
     11.2.0              | + y   | 11    | USE += "lto pch zstd (-cet)              |
                         |       |       |    (-custom-cflags) -d -valgrind"        |
     11.2.1_p20220115    | + y   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
 [I] 11.3.0              | + y   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
     11.3.1_p20220624    | o n   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
     11.4.9999           | o n   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
-------------------------+-------+-------+------------------------------------------|
     12.1.0              | o n   | 12    | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
     12.1.1_p20220528-r1 | o n   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
 [I] 12.1.1_p20220625    | ~ y   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
     12.2.9999           | o n   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
-------------------------+-------+-------+------------------------------------------|
     13.0.0_pre20220619  | o n   | 13    | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
     13.0.9999           | o n   |       | USE += "lto zstd (-cet)                  |
                         |       |       |    (-custom-cflags) -d (-pch) -valgrind" |
-------------------------+-------+-------+------------------------------------------|
#####################################################
Total time : 121ms
```

**Notes:**
- The masked versions are not yet put between parentheses, _e.g._ `(8.5.0-r1)` instead of the displayed `8.5.0-r1`.

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
