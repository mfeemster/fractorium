TEMPLATE = app
CONFIG += console

# Uncomment this if you only want to build a binary instead of an app bundle.
#macx:CONFIG -= app_bundle

CONFIG -= qt

TARGET = emberanimate

include(../defaults.pri)

#Point to visual studio resource file to embed file information and icon.
win32 {
    RC = $$RCPATH/EmberAnimate.rc
    win32:RC_FILE = $$RC
	#message(RC_FILE: $$RC)
}

#Go up one folder because the paths defined in defaults were relative to it, which is up one folder.
PRJ_SRC_DIR = $$absolute_path($$EMBER_ROOT/../Source/EmberAnimate)
#message(PRJ_SRC_DIR: $$PRJ_SRC_DIR)

target.path = $$BIN_INSTALL_DIR
#message(TARGET INSTALL: $$target.path)
INSTALLS += target

palettes.path = $$SHARE_INSTALL_DIR
palettes.files = $$ASSETS_DIR/flam3-palettes.xml
#message(PALETTE INSTALL SOURCE: $$palettes.files)
INSTALLS += palettes

!win32 {
	icon.path = $$SHARE_INSTALL_DIR
	icon.files = $$absolute_path($$PRJ_SRC_DIR/Icons/Fractorium.png)
	#message(ICON INSTALL SOURCE: $$icon.files)
	INSTALLS += icon

	launcher.path = $$LAUNCHER_INSTALL_DIR
	launcher.files = $$ASSETS_DIR/Fractorium.desktop
	#message(LAUNCHER INSTALL SOURCE: $$launcher.files)
	INSTALLS += launcher
}

macx:ICON = $$ASSETS_DIR/Fractorium.icns
!macx:PRECOMPILED_HEADER = $$SRC_COMMON_DIR/EmberCommonPch.h

LIBS += -L$$absolute_path($$DESTDIR) -lember
LIBS += -L$$absolute_path($$DESTDIR) -lembercl

SOURCES += \
    $$PRJ_SRC_DIR/EmberAnimate.cpp \
    $$SRC_COMMON_DIR/EmberCommonPch.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    $$PRJ_SRC_DIR/EmberAnimate.h \
    $$SRC_COMMON_DIR/EmberCommon.h \
    $$SRC_COMMON_DIR/EmberCommonPch.h \
    $$SRC_COMMON_DIR/EmberOptions.h \
    $$SRC_COMMON_DIR/JpegUtils.h \
    $$SRC_COMMON_DIR/SimpleGlob.h \
    $$SRC_COMMON_DIR/SimpleOpt.h

