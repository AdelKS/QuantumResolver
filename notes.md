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
