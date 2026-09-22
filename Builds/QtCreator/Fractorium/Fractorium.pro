TEMPLATE = app
QT += core gui widgets opengl openglwidgets concurrent

TARGET = fractorium

include(../defaults.pri)

#Point to visual studio resource file to embed file information and icon.
win32 {
    RC = $$RCPATH/Fractorium.rc
    win32:RC_FILE = $$RC
	#message(RC_FILE: $$RC)
}

#message(QTDIR: $$(QTDIR))

#Go up one folder because the paths defined in defaults were relative to it, which is up one folder.
PRJ_SRC_DIR = $$absolute_path($$EMBER_ROOT/../Source/Fractorium)
#message(PRJ_SRC_DIR: $$PRJ_SRC_DIR)

#Qt will be installed on a system wide level on *nix. It should be the same for Windows, but copy it local just to be safe.
win32 {
CONFIG(release, debug|release) {
	qtfiles.path = $$BIN_INSTALL_DIR
	
	qtfiles.files = $$(QTDIR)\bin\Qt6Core.dll $$(QTDIR)\bin\Qt6Gui.dll $$(QTDIR)\bin\Qt6OpenGL.dll $$(QTDIR)\bin\Qt6OpenGLWidgets.dll $$(QTDIR)\bin\Qt6Widgets.dll
	INSTALLS += qtfiles

	qtplatforms.path = $$BIN_INSTALL_DIR\platforms
	qtplatforms.files = $$(QTDIR)\plugins\platforms\qwindows.dll
	INSTALLS += qtplatforms
	
	qimageformats.path = $$BIN_INSTALL_DIR\imageformats
	qimageformats.files = $$(QTDIR)\plugins\imageformats\qjpeg.dll
	INSTALLS += qimageformats
}

CONFIG(debug, debug|release) {
	qtfiles.path = $$BIN_INSTALL_DIR
	qtfiles.files = $$(QTDIR)\bin\Qt6Cored.dll $$(QTDIR)\bin\Qt6Guid.dll $$(QTDIR)\bin\Qt6OpenGLd.dll $$(QTDIR)\bin\Qt6OpenGLWidgetsd.dll $$(QTDIR)\bin\Qt6Widgetsd.dll
	INSTALLS += qtfiles

	qtplatforms.path = $$BIN_INSTALL_DIR\platforms
	qtplatforms.files = $$(QTDIR)\plugins\platforms\qwindowsd.dll
	INSTALLS += qtplatforms
	
	qimageformats.path = $$BIN_INSTALL_DIR\imageformats
	qimageformats.files = $$(QTDIR)\plugins\imageformats\qjpeg.dll
	INSTALLS += qimageformats
}
}

#For some reason, a Qt project needs to be told to look at itself.
INCLUDEPATH += $$PRJ_SRC_DIR
INCLUDEPATH += $$PRJ_SRC_DIR/PaletteEditor
INCLUDEPATH += $$(QTDIR)\include\QtWidget

# Uncomment this if you only want to build a binary instead of an app bundle.
#macx:CONFIG -= app_bundle

target.path = $$BIN_INSTALL_DIR
#message(TARGET INSTALL: $$target.path)
INSTALLS += target

palettes.path = $$SHARE_INSTALL_DIR
palettes.files = $$ASSETS_DIR/flam3-palettes.xml \
$$ASSETS_DIR/*.gradient \
$$ASSETS_DIR/*.ugr

#message(PALETTE INSTALL SOURCE: $$palettes.files)
INSTALLS += palettes

themes.path = $$SHARE_INSTALL_DIR
win32 {
themes.files = $$ASSETS_DIR/dark_windows.qss
}

macx {
themes.files = $$ASSETS_DIR/dark_mac.qss
}

unix:!macx {
themes.files = $$ASSETS_DIR/dark_linux.qss
}

themes.files += $$ASSETS_DIR/lightdark.qss
themes.files += $$ASSETS_DIR/uranium.qss

#message(THEMES INSTALL SOURCE: $$themes.files)
INSTALLS += themes

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
!macx:PRECOMPILED_HEADER = $$PRJ_SRC_DIR/FractoriumPch.h

LIBS += -L$$absolute_path($$DESTDIR) -lember
LIBS += -L$$absolute_path($$DESTDIR) -lembercl

SOURCES += \
    $$PRJ_SRC_DIR/AboutDialog.cpp \
    $$PRJ_SRC_DIR/csshighlighter.cpp \
    $$PRJ_SRC_DIR/CurvesGraphicsView.cpp \
    $$PRJ_SRC_DIR/DoubleSpinBox.cpp \
    $$PRJ_SRC_DIR/FinalRenderDialog.cpp \
    $$PRJ_SRC_DIR/FinalRenderEmberController.cpp \
    $$PRJ_SRC_DIR/Fractorium.cpp \
    $$PRJ_SRC_DIR/FractoriumEmberController.cpp \
    $$PRJ_SRC_DIR/FractoriumInfo.cpp \
    $$PRJ_SRC_DIR/FractoriumLibrary.cpp \
    $$PRJ_SRC_DIR/FractoriumMenus.cpp \
    $$PRJ_SRC_DIR/FractoriumPalette.cpp \
    $$PRJ_SRC_DIR/FractoriumParams.cpp \
    $$PRJ_SRC_DIR/FractoriumPch.cpp \
    $$PRJ_SRC_DIR/FractoriumRender.cpp \
    $$PRJ_SRC_DIR/FractoriumSettings.cpp \
    $$PRJ_SRC_DIR/FractoriumToolbar.cpp \
    $$PRJ_SRC_DIR/FractoriumXaos.cpp \
    $$PRJ_SRC_DIR/FractoriumXformsAffine.cpp \
    $$PRJ_SRC_DIR/FractoriumXformsColor.cpp \
    $$PRJ_SRC_DIR/FractoriumXforms.cpp \
    $$PRJ_SRC_DIR/FractoriumXformsSelect.cpp \
    $$PRJ_SRC_DIR/FractoriumXformsVariations.cpp \
    $$PRJ_SRC_DIR/GLEmberController.cpp \
    $$PRJ_SRC_DIR/GLWidget.cpp \
    $$PRJ_SRC_DIR/Main.cpp \
    $$PRJ_SRC_DIR/OptionsDialog.cpp \
    $$PRJ_SRC_DIR/qcssparser.cpp \
    $$PRJ_SRC_DIR/qcssscanner.cpp \
    $$PRJ_SRC_DIR/QssDialog.cpp \
    $$PRJ_SRC_DIR/QssTextEdit.cpp \
    $$PRJ_SRC_DIR/SpinBox.cpp \
    $$PRJ_SRC_DIR/VariationsDialog.cpp \
    $$PRJ_SRC_DIR/LibraryTreeWidget.cpp \
    $$PRJ_SRC_DIR/PaletteEditor/ColorPanel.cpp \
    $$PRJ_SRC_DIR/PaletteEditor/ColorPickerWidget.cpp \
    $$PRJ_SRC_DIR/PaletteEditor/ColorTriangle.cpp \
    $$PRJ_SRC_DIR/PaletteEditor/GradientColorsView.cpp \
    $$PRJ_SRC_DIR/PaletteEditor/PaletteEditor.cpp

HEADERS += \
    $$SRC_COMMON_DIR/EmberCommon.h \
    $$SRC_COMMON_DIR/EmberCommonPch.h \
    $$SRC_COMMON_DIR/JpegUtils.h \
    $$PRJ_SRC_DIR/AboutDialog.h \
    $$PRJ_SRC_DIR/csshighlighter.h \
    $$PRJ_SRC_DIR/CurvesGraphicsView.h \
    $$PRJ_SRC_DIR/DoubleSpinBox.h \
    $$PRJ_SRC_DIR/DoubleSpinBoxTableItemDelegate.h \
    $$PRJ_SRC_DIR/EmberFile.h \
    $$PRJ_SRC_DIR/EmberTreeWidgetItem.h \
    $$PRJ_SRC_DIR/FinalRenderDialog.h \
    $$PRJ_SRC_DIR/FinalRenderEmberController.h \
    $$PRJ_SRC_DIR/FractoriumCommon.h \
    $$PRJ_SRC_DIR/FractoriumEmberController.h \
    $$PRJ_SRC_DIR/Fractorium.h \
    $$PRJ_SRC_DIR/FractoriumPch.h \
    $$PRJ_SRC_DIR/FractoriumSettings.h \
    $$PRJ_SRC_DIR/GLEmberController.h \
    $$PRJ_SRC_DIR/GLWidget.h \
    $$PRJ_SRC_DIR/OptionsDialog.h \
    $$PRJ_SRC_DIR/PaletteTableWidgetItem.h \
    $$PRJ_SRC_DIR/qcssparser.h \
    $$PRJ_SRC_DIR/qcssscanner.h \
    $$PRJ_SRC_DIR/qfunctions.h \
    $$PRJ_SRC_DIR/QssDialog.h \
    $$PRJ_SRC_DIR/QssTextEdit.h \
    $$PRJ_SRC_DIR/resource.h \
    $$PRJ_SRC_DIR/SpinBox.h \
    $$PRJ_SRC_DIR/StealthComboBox.h \
    $$PRJ_SRC_DIR/TableWidget.h \
    $$PRJ_SRC_DIR/TwoButtonComboWidget.h \
    $$PRJ_SRC_DIR/VariationsDialog.h \
    $$PRJ_SRC_DIR/VariationTreeWidgetItem.h \
    $$PRJ_SRC_DIR/LibraryTreeWidget.h \
    $$PRJ_SRC_DIR/PaletteEditor/ColorPanel.h \
    $$PRJ_SRC_DIR/PaletteEditor/ColorPickerWidget.h \
    $$PRJ_SRC_DIR/PaletteEditor/ColorTriangle.h \
    $$PRJ_SRC_DIR/PaletteEditor/GradientArrow.h \
    $$PRJ_SRC_DIR/PaletteEditor/GradientColorsView.h \
    $$PRJ_SRC_DIR/PaletteEditor/PaletteEditor.h

FORMS += \
    $$PRJ_SRC_DIR/AboutDialog.ui \
    $$PRJ_SRC_DIR/FinalRenderDialog.ui \
    $$PRJ_SRC_DIR/Fractorium.ui \
    $$PRJ_SRC_DIR/OptionsDialog.ui \
    $$PRJ_SRC_DIR/QssDialog.ui \
    $$PRJ_SRC_DIR/VariationsDialog.ui \
    $$PRJ_SRC_DIR/PaletteEditor.ui

OTHER_FILES += \
    $$PRJ_SRC_DIR/Fractorium.aps \
    $$PRJ_SRC_DIR/Fractorium.rc

RESOURCES += \
    $$PRJ_SRC_DIR/Fractorium.qrc

bench.path = $$SHARE_INSTALL_DIR/bench
bench.files += \
    $$ASSETS_DIR/Bench/EmberBench.ps1 \
    $$ASSETS_DIR/Bench/*.flame

#message(BENCH INSTALL SOURCE: $$bench.files)
INSTALLS += bench

examples.path = $$SHARE_INSTALL_DIR/examples
examples.files += \
    $$ASSETS_DIR/examples/*.flame

#message(EXAMPLES INSTALL SOURCE: $$examples.files)
INSTALLS += examples

