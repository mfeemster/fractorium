VERSION = 25.25.12.1
win32:CONFIG += skip_target_version_ext
CONFIG += c++2a

#message(PWD: $$absolute_path($$PWD))

#1) Declare the root of all files in this project, everything else will be
#   defined in terms of this.
EMBER_ROOT = ./../..

# When compiling from project root
autobuild {
#  EMBER_ROOT = $$(PWD)/../..
}

#2) Declare where dependency folders are.
#   Parent folders for third party dependencies and their compiled outputs.
win32: {
	EXTERNAL_DIR = $$absolute_path($$EMBER_ROOT/..)
	EXTERNAL_LIB = $$absolute_path($$EMBER_ROOT/Deps)
}

#3) Declare where our source, data and resource files are.
SRC_DIR = $$EMBER_ROOT/Source
SRC_COMMON_DIR = $$absolute_path($$EMBER_ROOT/Source/EmberCommon)
ASSETS_DIR = $$absolute_path($$EMBER_ROOT/Data)
QTCREATOR_DIR = $$absolute_path($$EMBER_ROOT/Builds/QtCreator)
win32:RCPATH=$$absolute_path($$QTCREATOR_DIR/../MSVC/Solution)

#4) Add up all include paths.
INCLUDEPATH += $$LOCAL_INCLUDE_DIR
INCLUDEPATH += $$absolute_path($$SRC_DIR/Ember)
INCLUDEPATH += $$absolute_path($$SRC_DIR/EmberCL)
macx {
INCLUDEPATH += $$absolute_path($$EMBER_ROOT/Builds/include/CLs/CL1.2)
}
else {
INCLUDEPATH += $$absolute_path($$EMBER_ROOT/Builds/include/CLs/CL3.0)
}
INCLUDEPATH += $$absolute_path($$SRC_DIR/EmberCommon)

win32 {
	INCLUDEPATH += $$EXTERNAL_DIR/glm
	INCLUDEPATH += $$EXTERNAL_DIR/libjpeg
	INCLUDEPATH += $$EXTERNAL_DIR/libpng
	INCLUDEPATH += $$EXTERNAL_DIR/libxml2/include
	INCLUDEPATH += $$EXTERNAL_DIR/zlib
	INCLUDEPATH += $$EXTERNAL_DIR/openexr/output/include/Imath
	INCLUDEPATH += $$EXTERNAL_DIR/openexr/output/include/OpenEXR
}

!win32 {
#If your global includes are stored elsewhere, add them here.
	#INCLUDEPATH += /usr/include
	INCLUDEPATH += /usr/local/include
	INCLUDEPATH += /usr/include/GL
exists( /usr/local/include/GL ) {
	INCLUDEPATH += /usr/local/include/GL
}
	INCLUDEPATH += /usr/include/glm
exists( /usr/local/include/GL ) {
	INCLUDEPATH += /usr/include/Imath
}
exists( /usr/include/OpenEXR ) {
	INCLUDEPATH += /usr/include/OpenEXR
}
exists( /usr/include/Imath ) {
	INCLUDEPATH += /usr/include/Imath
}

unix:!macx {
	INCLUDEPATH += /usr/include/libxml2
}
else {
	INCLUDEPATH += /usr/local/opt/ilmbase/include/OpenEXR
	INCLUDEPATH += /usr/local/opt/openexr@2/include
	INCLUDEPATH += /usr/local/opt/jpeg/include
}

#libjpeg and libpng aren't in separate folders, so nothing to add here for them.
}

#5) Add up all library paths. Ember and EmberCL don't need libjpeb, libpng or zlib and
# Ember doesn't need OpenCL. But just place them all here in the common file for ease of maintenance.
# Unneeded libs will just be ignored.
win32 {
	LIBS = ""
	LIBS += OpenGL32.lib
	LIBS += WS2_32.lib
	LIBS += $$(OCL_ROOT)/lib/x86_64/OpenCL.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/libjpeg.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/libpng.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/libxml2.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/zlib.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/Iex-3_1.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/IlmThread-3_1.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/Imath-3_1.lib
	LIBS += $$absolute_path($$EXTERNAL_LIB)/OpenEXR-3_1.lib
}

!win32 {
	LIBS += -ljpeg
	LIBS += -lpng
	LIBS += -lxml2
	LIBS += -lz
	LIBS += -lpthread
	LIBS += -lImath
	LIBS += -lIex

#For systems with older versions of OpenEXR < 3.0, use this.
#	LIBS += -lIlmImf
#    LIBS += -lHalf

#For systems with OpenEXR >= 3.0, use this.
   LIBS += -lz
   LIBS += -lIlmThread
   LIBS += -lOpenEXR
}

macx {
	LIBS += -framework OpenGL
	LIBS += -framework OpenCL
	LIBS += -L/usr/local/lib# homebrew installs into /usr/local
	LIBS += -L/usr/local/opt/jpeg/lib
	LIBS += -L/usr/local/opt/openexr@2/lib
	LIBS += -L/usr/local/opt/ilmbase/lib
}

unix:!macx {
	LIBS += -lGL
	LIBS += -lOpenCL
}

#6) Declare intermediate paths.



#7) Declare output paths for each configuration.
CONFIG(release, debug|release) {
	CONFIG += warn_off
	DESTDIR = $$absolute_path($$EMBER_ROOT/Bin/release)
}

CONFIG(debug, debug|release) {
	DESTDIR = $$absolute_path($$EMBER_ROOT/Bin/debug)
}

#8) Set compiler options.
QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS_RELEASE += -DNDEBUG
QMAKE_CXXFLAGS += -D_M_X64
QMAKE_CXXFLAGS += -D_CONSOLE
QMAKE_CXXFLAGS += -D_USRDLL
QMAKE_CXXFLAGS -= -D_UNICODE
QMAKE_CXXFLAGS -= -DUNICODE
DEFINES -= _UNICODE
DEFINES -= UNICODE

win32 {
	QMAKE_CXXFLAGS += -bigobj #Allow for very large object files.
	QMAKE_CXXFLAGS += /MP #Enable multi-processor compilation.
	QMAKE_CXXFLAGS += /Zc:wchar_t #Treat wchar_t as builtin type (we don't use wchar_t anyway).
	QMAKE_CXXFLAGS += /Zi #Debug information format: program database.
	QMAKE_CXXFLAGS += /Gm- #Disable minimal rebuild, needed to allow /MP.
	QMAKE_CXXFLAGS += /fp:precise #Precise floating point model.
	QMAKE_CXXFLAGS += /fp:except- #Disable floating point exceptions.
	QMAKE_CXXFLAGS += /D "WIN32"
	QMAKE_CXXFLAGS += /D "_WINDOWS"
	QMAKE_CXXFLAGS += /D "_USRDLL"
	QMAKE_CXXFLAGS += /D "_WINDLL" #Build as a DLL.
	QMAKE_CXXFLAGS += /D "_MBCS" #Use multi-byte character set.
	QMAKE_CXXFLAGS += /errorReport:prompt #Internal compiler error reporting, prompt immediately.
	QMAKE_CXXFLAGS += /GF #Enable string pooling.
	QMAKE_CXXFLAGS += /WX- #Don't treat warnings as errors.
	QMAKE_CXXFLAGS += /Zc:forScope #Force conformance in for loop scope.
	QMAKE_CXXFLAGS += /Gd #Calling convention: __cdecl.
	QMAKE_CXXFLAGS += /EHsc #Enable C++ exceptions.
	QMAKE_CXXFLAGS += /nologo #Suppress compiler startup banner.

	QMAKE_CXXFLAGS_RELEASE += /GS- #Disable security check.
	QMAKE_CXXFLAGS_RELEASE += /MD #Link to multi-threaded DLL.
	QMAKE_CXXFLAGS_RELEASE += /Gy #Enable function level linking.
	QMAKE_CXXFLAGS_RELEASE += /O2 #Maximize speed.
	QMAKE_CXXFLAGS_RELEASE += /Ot #Favor fast code.
	QMAKE_CXXFLAGS_RELEASE += /D "NDEBUG" #Release mode.

	QMAKE_CXXFLAGS_DEBUG += /W3 #Error warning level to 3.
	QMAKE_CXXFLAGS_DEBUG += /GS #Enable security check.
	QMAKE_CXXFLAGS_DEBUG += /MDd #Link to multi-threaded debug DLL.
	QMAKE_CXXFLAGS_DEBUG += /Od #Optimization disabled.
	QMAKE_CXXFLAGS_DEBUG += /D "_DEBUG" #Debug mode.
	QMAKE_CXXFLAGS_DEBUG += /RTC1 #Basic runtime checks: stack frames and uninitialized variables.
    QMAKE_CXXFLAGS_DEBUG += /Ob2 #Inline function expansion: any suitable.
}

!win32 {
	native {
		QMAKE_CXXFLAGS += -march=native
	} else {
		QMAKE_CXXFLAGS += -march=k8
	}

	QMAKE_CXXFLAGS_RELEASE += -fomit-frame-pointer
	QMAKE_CXXFLAGS += -fPIC
	QMAKE_CXXFLAGS += -fpermissive
	QMAKE_CXXFLAGS += -pedantic
    QMAKE_CXXFLAGS += -std=c++2a
	QMAKE_CXXFLAGS += -Wnon-virtual-dtor
	QMAKE_CXXFLAGS += -Wshadow
	QMAKE_CXXFLAGS += -Winit-self
	QMAKE_CXXFLAGS += -Wredundant-decls
	QMAKE_CXXFLAGS += -Wcast-align
	QMAKE_CXXFLAGS += -Winline
	QMAKE_CXXFLAGS += -Wunreachable-code
	QMAKE_CXXFLAGS += -Wswitch-enum
	QMAKE_CXXFLAGS += -Wswitch-default
	QMAKE_CXXFLAGS += -Wmain
	QMAKE_CXXFLAGS += -Wfatal-errors
	QMAKE_CXXFLAGS += -Wall -fpermissive

        # CL has lots of old-style-cast which clogs up warning logs
	#QMAKE_CXXFLAGS += -Wold-style-cast

	QMAKE_CXXFLAGS += -Wno-unused-parameter
	QMAKE_CXXFLAGS += -Wno-unused-function
	QMAKE_CXXFLAGS += -Wold-style-cast

	QMAKE_CXXFLAGS_DEBUG += -Wmissing-include-dirs
	QMAKE_CXXFLAGS_DEBUG += -Wzero-as-null-pointer-constant
# NOTE: last path will be the first to search. gcc -I and -L appends to the
# beginning of the path list.

# NOTE: qmake will resolve symlinks. If /usr/local/include/CL is a symlink to
# /usr/include/nvidia-352/CL, qmake will generate Makefiles using the latter.
}

macx {
	QMAKE_MAC_SDK = macosx12.1
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 11.0
	QMAKE_CXXFLAGS += -mmacosx-version-min=11.0 -arch x86_64
	QMAKE_CXXFLAGS += -stdlib=libc++
}

unix {
	CONFIG += precompile_header
	QMAKE_LFLAGS_RELEASE += -s
}

#9) Declare !win32 install dirs.
win32 {#For Windows, the install folder is just the output folder.
	LIB_INSTALL_DIR = $$DESTDIR
	BIN_INSTALL_DIR = $$DESTDIR
	SHARE_INSTALL_DIR = $$DESTDIR
	LAUNCHER_INSTALL_DIR = $$DESTDIR
}

!win32 {
	LIB_INSTALL_DIR = /usr/lib
	BIN_INSTALL_DIR = /usr/bin
	SHARE_INSTALL_DIR = /usr/share/fractorium
	LAUNCHER_INSTALL_DIR = /usr/share/applications
}

#10) Add third party libraries to install dir.
win32 {
	libxml.path = $$BIN_INSTALL_DIR
	libxml.files = $$absolute_path($$EMBER_ROOT/Deps/libxml2.dll)
	INSTALLS += libxml

    iex.path = $$BIN_INSTALL_DIR
    iex.files = $$absolute_path($$EMBER_ROOT/Deps/Iex-3_1.dll)
    INSTALLS += iex

    imath.path = $$BIN_INSTALL_DIR
    imath.files = $$absolute_path($$EMBER_ROOT/Deps/Imath-3_1.dll)
    INSTALLS += imath

    ilmthread.path = $$BIN_INSTALL_DIR
    ilmthread.files = $$absolute_path($$EMBER_ROOT/Deps/IlmThread-3_1.dll)
    INSTALLS += ilmthread

    openexr.path = $$BIN_INSTALL_DIR
    openexr.files = $$absolute_path($$EMBER_ROOT/Deps/OpenEXR-3_1.dll)
    INSTALLS += openexr
}

#11) Print values of relevant variables for debugging.
#message(CONFIG: $(CONFIG))
#message(EMBER_ROOT: $$absolute_path($$EMBER_ROOT))
#message(EXTERNAL_DIR: $$absolute_path($$EXTERNAL_DIR))
#message(EXTERNAL_LIB: $$absolute_path($$EXTERNAL_LIB))
#message(SRC_DIR: $$absolute_path($$SRC_DIR))
#message(SRC_COMMON_DIR: $$SRC_COMMON_DIR)
#message(ASSETS_DIR: $$absolute_path($$ASSETS_DIR))
#message(LOCAL_INCLUDE_DIR: $$absolute_path($$LOCAL_INCLUDE_DIR))
#message(QTCREATOR_DIR: $$absolute_path($$QTCREATOR_DIR))
#message(LIBS: $$absolute_path($$LIBS))
#message(DESTDIR: $$absolute_path($$DESTDIR))
#message(DEPENDPATH: $$absolute_path($$DEPENDPATH))
#message(AMDAPPSDKROOT: $$(AMDAPPSDKROOT))
#message(CUDA_PATH: $$(CUDA_PATH))

win32 {
	#message(RCPATH: $$RCPATH)
}

#message(LIB_INSTALL_DIR: $$absolute_path($$LIB_INSTALL_DIR))
#message(BIN_INSTALL_DIR: $$absolute_path($$BIN_INSTALL_DIR))
#message(SHARE_INSTALL_DIR: $$absolute_path($$SHARE_INSTALL_DIR))
#message(LAUNCHER_INSTALL_DIR: $$absolute_path($$LAUNCHER_INSTALL_DIR))
