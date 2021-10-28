### Quantum Resolver

This is an attempt at writing a faster Gentoo dependency resolver with a hope to get it upstreamed and replace that specific part in `emerge`. This project is at its early beginnings.

The current main ideas:
- Map each package name and use flag to a unique integer, for example `sys-devel/gcc` -> `13`, `pgo` -> `13` (use flags do not live in the same world as package names), and work only with integers while resolving dependencies.
  - For this purpose, implemented the class [IndexedVector](src/indexedvector.h) that enables accessing objects with an integer index and another type of index. It will be used to access package information through either their name, e.g. `sys-devel/gcc` or their index `13`.
- Parse the initial files on disk only once and produce a fast intermediate representation that helps speeding up dependency resolution
  - This has been done for now for `ebuild` versions: see [ebuildversion.cpp](src/ebuildversion.cpp)


#### How to ebuild

In its current state, you need `qmake`

```shell
qmake QuantumResolver.pro
make
```

For a debug build

```shell
qmake CONFIG+=debug QuantumResolver.pro
make
```

Otherwise, if you have `QtCreator` you can simply open the `.pro` file and setup the project for "Release" and "Debug" builds. Then press the "Play" button.
