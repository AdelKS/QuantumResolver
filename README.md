### Quantum Resolver

This is an attempt at writing a faster Gentoo dependency resolver with a hope to get it upstreamed and replace that specific part in `emerge`. This project is at its early beginnings.

The current main ideas:
- Map each package name and use flag to a unique integer, for example `sys-devel/gcc` -> `13`, `pgo` -> `13`, and work only with integers while resolving dependencies.
  - For this purpose, implemented the class [IndexedVector](src/indexedvector.h) that enables accessing objects with an integer index and another index. It will be used to access package information through either their name, e.g. `sys-devel/gcc` or their index `13`.
- Parse the initial files on disk only once and produce a fast intermediate representation that helps speeding up dependency resolution
  - This has been done for now for `ebuild` versions: see [ebuildversion.cpp](src/ebuildversion.cpp)
