- Expanded variables in EBUILDs: /var/db/repos/gentoo/metadata/md5-cache/
    - Update repository md5-cache: ```sudo egencache --update --repo gentoo```
- Installed packages and the USE flags used: /var/db/pkg
- Profile use flags: /etc/portage/make.profile
- To get system-wide use flags: ```portageq envvar USE```, slower and with extra clutter: ```emerge --info```

TODO:
- useflag reading routines
    - read per-package flags and process them so they can be properly set
- dependency graphs
    - do the simple approach where all or and useflag conditions are omitted and dependencies are just plain
