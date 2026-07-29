Fractorium
==========

A Qt-based fractal flame editor which uses a C++ re-write of the flam3 algorithm
named Ember and a GPU capable version named EmberCL which implements a portion
of the cuburn algorithm in OpenCL.

# Installing

## Windows

Download: [Fractorium_25.25.12.1.msi](https://drive.google.com/file/d/1LZHAR2j5I09qf_tBBBb_sxD0uVR_UEwo/view?usp=sharing)

## Mac

Download: [Fractorium_25.25.12.1.dmg](https://drive.google.com/file/d/1LcqFFiDwgjKOrNtf9Ce8l7fLn5xlVRzm/view?usp=sharing)

## Linux

Fractorium is supported on Ubuntu and Red Hat linux distributions. It is not supported on Arch or any other distributions due to severe unaddressed issues with AppImage. If you need to run it elsewhere, please build from source.

### Install App Image from .deb

Install ubuntu 20 or greater.

Download: [Fractorium-25.25.12.1.x86_64.deb](https://drive.google.com/file/d/1GIwCinL6T81J9gq39-zvoMo8vcJwqIly/view?usp=sharing)

```
cd ~/Downloads
sudo dpkg -i Fractorium-25.25.12.1.x86_64.deb
```

### Install App Image .rpm

Download: [Fractorium-25.25.12.1.x86_64.rpm](https://drive.google.com/file/d/1eFynIUYNW-a9BjxWoeRrty0ym0Slzg3m/view?usp=sharing)

### Final configuration


If you don't have OpenCL installed on your system, you need to run this to enable OpenCL support:

`sudo apt install mesa-opencl-icd`

If your system is using rusticl then you need to add this environment variable in `~/.profile` to allow double precision support:

`export RUSTICL_FEATURES=fp64`

Without it, douple precision mode will fail to compile in OpenCL and revert to the CPU renderer. Additionally, some variations such as Elliptic will fail even in single precision mode.

# Building from git

All builds are 64-bit.

## Windows

[Build Guide for Visual Studio 2026 or Qt Creator](Data/BuildGuideQtCreator.md)

## Linux

[Build Guide for Linux](./Data/BuildGuideLinux.md)

## Mac OS/X Sierra, El Capitan, Yosemite and Mavericks

[Build Guide for Mac OS/X](./Data/BuildGuideMacOSX.md)
