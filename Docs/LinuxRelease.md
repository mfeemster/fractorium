# Linux Release

These are instructions for building a Linux AppImage and deb/rpm packages which contain the AppImage.
They are only supported on Ubuntu 24+, such as Mint 22+.

## Summary

The following instructions assume you have installed all of the dependencies mentioned in the linux build guide.

Clone the repository:

```
$ git clone https://github.com/mfeemster/fractorium.git
$ cd fractorium
```

Assuming you installed Qt in a location like:
`~/Dev/Qt/6.11.2`

Go into `dist/scripts/build_and_package_all.sh` and change the `QT_PREFIX` variable to point to your Qt installation like:
`QT_PREFIX="${QT_PREFIX:-$HOME/Dev/Qt/6.11.2/gcc_64}"`

Also change the `APP_VERSION` variable to match the version specified in `EmberDefines.h`.

Go into `/Builds/QtCreator/defaults.pri` and ensure that version also matches.

Go into `dist/debian/changelog` and add a new entry. Set the timestamp with `date -R`.

Everything is now ready to build.
Run the build and packaging script:
```
dist/scripts/build_and_package_all.sh
```
This will build the entire solution then package it into an AppImage, a deb and and an RPM.
When the process completes, you will see the following binaries in `dist/`:
```
Fractorium-[w.x.y.z].x86_64.AppImage
Fractorium-[w.x.y.z].x86_64.deb
Fractorium-[w.x.y.z].x86_64.rpm
```

The AppImage file can be run just by double clicking on it. However, keep in mind this does not install the command line program shortcuts, so you always have to run from the AppImage file. The palette and example files are mounted internally and the `.qss` files are copied to `~/.config/fractorium`.

For a full install which contains the AppImage file and the shortcuts to it, use either the `.deb` for Ubuntu systems or the `.rpm` for Red Hat systems.
* These will create shortcuts for `fractorium`, `emberrender`, `emberanimate` and `embergenome` which call into the installed AppImage file.