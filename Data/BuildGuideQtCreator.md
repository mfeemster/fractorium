# Build Guide For Visual Studio 2026 (or later) or Qt Creator
## Tools

### git

Install [git](https://git-scm.com/downloads).

### Visual Studio

Install [Microsoft Visual Studio 2026 or later](https://www.visualstudio.com/downloads/), then install the latest updates.

### Qt

Install Qt for Windows 64-bit 6.5.1 or later (http://www.qt.io/download/). 

Add system environment variable named `QTPATH` and point it to the location of the Qt folder. On a default install, this will be something like:

`C:\Qt\6.11.2\msvc2022_64`

### Wix

To build the installer, you must have Wix installed as well as the Heatwave VS extension. If you are unconcerned with it, you can skip this step and just dismiss the warning that shows when opening the solution later. It's recommended you ignore the installer since official builds are provided on this page.

### CMake

Install [CMake](https://cmake.org/download/) and be sure to check the option to add it to the system path.

## Obtaining source

### This project

Open up the Visual Studio x64 Native Tools Command Prompt.

Create a new folder in your development area named fractorium:

```
mkdir fractorium
cd fractorium
git clone https://github.com/mfeemster/fractorium.git
```

### Prerequisites

There are six prerequisite dependencies. One of them must be downloaded manually:

Download the zip file here: [libjpeg](http://www.ijg.org/).

Extract the source contents into the folder you created such that it is arranged like so:

```
[fractorium]
│  
├─libjpeg
```

Go into the fractorium folder and run this script which will get the rest of the prerequisites from git and build them:

`cd fractorium`

`makedeps.bat`

This will download and build `glm libopenexr libpng libxml zlib`. You will have a folder structure like this:

```
[fractorium]
│  
├─[glm]
├─[libjpeg]
├─[libpng]
├─[libxml2]
├─[openexr]
└─[fractorium]
  │
  ├─[Deps]
    │
    ├─Iex-3_4.dll
    ├─IlmThread-3_4.dll
    ├─IMath-3_2.dll
    ├─libxml2.dll
    ├─OpenEXR-3_4.dll
    ├─OpenEXRCore-3_4.dll
    ├─openjph.0.24.dll
    ├─Iex-3_4.lib
    ├─IlmThread-3_4.lib
    ├─Imath-3_2.lib
    ├─libjpeg.lib
    ├─libpng.lib
    ├─libxml2.lib
    ├─OpenEXR-3_4.lib
    ├─OpenEXRCore-3_4.lib
    ├─OpenEXRUtil-3_4.lib
    ├─openjph.0.24.lib
    ├─zlib.lib
```

## Building with Qt Creator or Visual Studio

### Begin build with Qt Creator

Open the Qt Project `fractorium/main.pro` using Qt Creator with the default config of *Desktop Qt [version] MSVC2022 64bit*.
Make sure *Shadow build* in *Edit build configuration* for both *Debug* and *Release* is unchecked.

Switch to the *Release* configuration.

Under *Projects* -> *Build Steps*, add an additional argument of `install` to the `make`/`jom` command to force all dependencies to be copied to the output folder. The final make command should look like:

`jom.exe install in /path/to/fractorium`

Ensure all projects are in the *Release* configuration and build main.pro.

The outputs will be placed in `fractorium/Bin/release` several minutes later if no error occurs.

### Begin build with Visual Studio

#### Visual Studio Qt Addon

Install the Visual Studio Qt Addon extension.

Click *Extensions* -> *Qt VS Tools* -> *Qt Versions* and point it to your installation. Autodetect usually works well.

Open the file Fractorium.sln under Builds/MSVC/Solution

Set the configuration to release, and build all.

### Outputs

The outputs will be the same whether Visual Studio or Qt Creator was used, however their locations will be different.

Qt Creator will place its outputs in:

`fractorium/Bin/Release`

and Visual Studio will place its outputs in:

`fractorium/Bin/x64/Release`

The output contents will be:

```
Amphibole_Supergroup.ugr
Apatite_Supergroup.ugr
boxtail_pack_02.gradient
boxtail_pack_03_triangle.gradient
boxtail_pack_04_mineshack.gradient
dark_windows.qss
Ember.dll
EmberAnimate.exe
EmberCL.dll
EmberGenome.exe
EmberRender.exe
fardareismai_pack_01_variety_number_128.gradient
fardareismai_pack_02_b_sides.gradient
fardareismai_pack_03_old_and_new.gradient
fardareismai_pack_04_hoard.gradient
Feldspar_Group.ugr
flam3-palettes.xml
fractaldesire_pack_01.gradient
Fractorium.exe
Iex-3_4.dll
IlmThread-3_4.dll
Imath-3_2.dll
libxml2.dll
lightdark.qss
Mica_Group.ugr
OpenEXR-3_4.dll
Qt6Core.dll
Qt6Gui.dll
Qt6OpenGL.dll
Qt6OpenGLWidgets.dll
Qt6Widgets.dll
Quartz_Varieties.ugr
rce_ordinary_pack_01_colornation.gradient
tatasz_pack_01.gradient
tatasz_pack_02_colder.gradient
tatasz_pack_02_dark.gradient
tatasz_pack_02_warmer.gradient
tatasz_pack_03.gradient
tatasz_pack_04.gradient
tatasz_pack_05.gradient
tatasz_pack_06.gradient
tatasz_pack_07.gradient
uranium.qss
user-palettes.xml
imageformats\qjpeg.dll
platforms\qwindows.dll
```

Double click fractorium.exe to run it, and use the command line to run the others.

To run on a computer without Visual Studio 2026 (or later), these files also need to be in the folder:

```
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\[LATEST_VERSION]\x64\Microsoft.VC[LATEST_VERSION].CRT\msvcp140.dll
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\[LATEST_VERSION]\x64\Microsoft.VC[LATEST_VERSION].CRT\vcruntime140.dll
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\[LATEST_VERSION]\x64\Microsoft.VC[LATEST_VERSION].CRT\vcruntime140_1.dll
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\[LATEST_VERSION]\x64\Microsoft.VC[LATEST_VERSION].CRT\vccorlib140.dll
C:\Program Files\Microsoft Visual Studio\18\Community\VC\Redist\MSVC\[LATEST_VERSION]\x64\Microsoft.VC[LATEST_VERSION].CRT\concrt140.dll
```

or you can install [Visual C++ Redistributable Packages for Visual Studio 2026 (or later) (64 bit)](https://www.visualstudio.com/downloads/)

## Final file structure for distribution

```
[YOUR FOLDER]
│
├─Amphibole_Supergroup.ugr
├─Apatite_Supergroup.ugr
├─boxtail_pack_02.gradient
├─boxtail_pack_03_triangle.gradient
├─boxtail_pack_04_mineshack.gradient
├─dark_windows.qss
├─Ember.dll
├─EmberAnimate.exe
├─EmberCL.dll
├─EmberGenome.exe
├─EmberRender.exe
├─fardareismai_pack_01_variety_number_128.gradient
├─fardareismai_pack_02_b_sides.gradient
├─fardareismai_pack_03_old_and_new.gradient
├─fardareismai_pack_04_hoard.gradient
├─Feldspar_Group.ugr
├─flam3-palettes.xml
├─fractaldesire_pack_01.gradient
├─Fractorium.exe
├─Iex-3_4.dll
├─IlmThread-3_4.dll
├─Imath-3_2.dll
├─libxml2.dll
├─lightdark.qss
├─Mica_Group.ugr
├─OpenEXR-3_4.dll
├─Qt6Core.dll
├─Qt6Gui.dll
├─Qt6OpenGL.dll
├─Qt6OpenGLWidgets.dll
├─Qt6Widgets.dll
├─Quartz_Varieties.ugr
├─rce_ordinary_pack_01_colornation.gradient
├─tatasz_pack_01.gradient
├─tatasz_pack_02_colder.gradient
├─tatasz_pack_02_dark.gradient
├─tatasz_pack_02_warmer.gradient
├─tatasz_pack_03.gradient
├─tatasz_pack_04.gradient
├─tatasz_pack_05.gradient
├─tatasz_pack_06.gradient
├─tatasz_pack_07.gradient
├─uranium.qss
├─user-palettes.xml
├─msvcp140.dll (optional)
├─vcruntime140.dll (optional)
├─vcruntime140_1.dll (optional)
├─vccorlib140.dll (optional)
├─concrt140.dll (optional)  
└─[imageformats]
  │
  ├─qjpeg.dll
  └─[platforms]
    │
    ├─qwindows.dll
```

