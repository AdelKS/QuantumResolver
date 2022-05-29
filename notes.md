- Expanded variables in EBUILDs: /var/db/repos/gentoo/metadata/md5-cache/
    - Update repository md5-cache: ```sudo egencache --update --repo gentoo```
- Installed packages and the USE flags used: /var/db/pkg
- Profile use flags: /etc/portage/make.profile
- To get system-wide use flags: ```portageq envvar USE```, slower and with extra clutter: ```emerge --info```
- example package constraint:
    - `>=xfce-base/libxfce4ui-4.12:=[gtk3(+)]`
    - `>=x11-libs/gtk+-3.22:3[introspection?,X]`


TODO:
- Finish use_database
- Read /var/db/pkg for installed packages
    - to see change of use flags and mark ebuilds that have flag change
    - mark which version is installed
- Read world set of user installed packages
- Read system set: compare with world to understand better

#### Dependencies to satisfy per phase

(Interpreted from the [PMS](https://dev.gentoo.org/~ulm/pms/head/pms.html))

| Phase function | Satisfied dependency classes |
| -------------- | ---------------------------- |
| Prepare, compile and create an image, aka all the `src_*` functions | `DEPEND`, `BDEPEND`  |
| Interaction with the live filesystem: install or uninstall | `RDEPEND`, `IDEPEND` |
| `pkg_config`, after install | `RDEPEND`, `PDEPEND` |

The call order for installing a package is:

- Prepare, compile and create an image:
  1. `pkg_setup`
  2. `src_unpack`
  3. `src_prepare`
  4. `src_configure`
  5. `src_compile`
  6. `src_test` (except if RESTRICT=test or disabled by user)
  7. `src_install`
- Interaction with the live filesystem: install
  1. `pkg_preinst` : can touch stuff under `${ROOT}` (which usually points to `/`)
  2. [ ... emerge performs the install ... ]
  3. `pkg_postinst` : can touch stuff under `${ROOT}` (which usually points to `/`)

The call order for uninstalling a package is:

1. `pkg_prerm`
2. [ ... emerge performs the uninstall ... ]
3. `pkg_postrm`

The call order for upgrading, downgrading or reinstalling a package is:

- Prepare, compile and create an image:
  1. `pkg_setup`
  2. `src_unpack`
  3. `src_prepare`
  4. `src_configure`
  5. `src_compile`
  6. `src_test` (except if RESTRICT=test or disabled by user)
  7. `src_install`
- Interaction with the live filesystem: uninstall old then install new
  1. `pkg_preinst`
  2. `pkg_prerm` for the package that will be replaced
  3. [ ... emerge uninstalls old package ... ]
  4. `pkg_postrm` for the package that will be replaced
  5. [ ... emerge install new package ... ]
  6. `pkg_postinst` for the replacing package

For installing binary packages, the src phases are not called.

When building binary packages that are not to be installed locally, the `pkg_preinst` and `pkg_postinst` functions are not called.