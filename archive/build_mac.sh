#!/bin/bash

OSX_BUILD_PATH=$PWD
FRACTORIUM_RELEASE_ROOT=$PWD/../Bin/release

# Replace 6.11.3 with your Qt version, and ensure the installation path is the same
QT_DIR=~/Qt/6.11.3/macos
QT_MACDEPLOY=$QT_DIR/bin/macdeployqt6
QT_LIB_PATH=$QT_DIR/lib

cd $FRACTORIUM_RELEASE_ROOT

EMBERANIMATE_FINAL_ROOT=$PWD/emberanimate.app/Contents/MacOS
EMBERGENOME_FINAL_ROOT=$PWD/embergenome.app/Contents/MacOS
EMBERRENDER_FINAL_ROOT=$PWD/emberrender.app/Contents/MacOS
FRACTORIUM_FINAL_ROOT=$PWD/fractorium.app/Contents/MacOS
FRACTORIUM_FINAL_FRAMEWORKS=$PWD/fractorium.app/Contents/Frameworks

# 1. Update local dylib IDs and dependency paths
install_name_tool -id $PWD/libember.dylib libember.dylib
install_name_tool -id $PWD/libembercl.dylib libembercl.dylib
install_name_tool -change libember.dylib $PWD/libember.dylib libembercl.dylib

# Ensure dylibs can resolve Qt 6.11 frameworks during deployment
install_name_tool -add_rpath $QT_LIB_PATH libember.dylib 2>/dev/null || true
install_name_tool -add_rpath $QT_LIB_PATH libembercl.dylib 2>/dev/null || true

# 2. Copy helper executables into fractorium.app BEFORE running macdeployqt6
cp $EMBERANIMATE_FINAL_ROOT/emberanimate $FRACTORIUM_FINAL_ROOT/
cp $EMBERGENOME_FINAL_ROOT/embergenome $FRACTORIUM_FINAL_ROOT/
cp $EMBERRENDER_FINAL_ROOT/emberrender $FRACTORIUM_FINAL_ROOT/

# 3. Update library references and rpaths for all binaries inside fractorium.app
for BIN in fractorium emberanimate embergenome emberrender; do
    install_name_tool -change libember.dylib $PWD/libember.dylib $FRACTORIUM_FINAL_ROOT/$BIN
    install_name_tool -change libembercl.dylib $PWD/libembercl.dylib $FRACTORIUM_FINAL_ROOT/$BIN
    # Add Qt SDK rpath for deployment resolution and final bundle Frameworks rpath
    install_name_tool -add_rpath $QT_LIB_PATH $FRACTORIUM_FINAL_ROOT/$BIN 2>/dev/null || true
    install_name_tool -add_rpath @executable_path/../Frameworks $FRACTORIUM_FINAL_ROOT/$BIN 2>/dev/null || true
done

# 4. Run macdeployqt6 ONCE on fractorium.app, passing the extra executables and Qt libpath
$QT_MACDEPLOY fractorium.app \
    -libpath=$QT_LIB_PATH \
    -executable=$FRACTORIUM_FINAL_ROOT/emberanimate \
    -executable=$FRACTORIUM_FINAL_ROOT/embergenome \
    -executable=$FRACTORIUM_FINAL_ROOT/emberrender

# 5. Remove temporary local Qt SDK rpath so the bundle remains completely self-contained
for BIN in fractorium emberanimate embergenome emberrender; do
    install_name_tool -delete_rpath $QT_LIB_PATH $FRACTORIUM_FINAL_ROOT/$BIN 2>/dev/null || true
done
install_name_tool -delete_rpath $QT_LIB_PATH $FRACTORIUM_FINAL_FRAMEWORKS/libember.dylib 2>/dev/null || true
install_name_tool -delete_rpath $QT_LIB_PATH $FRACTORIUM_FINAL_FRAMEWORKS/libembercl.dylib 2>/dev/null || true

cd $OSX_BUILD_PATH
cd ../Data

cp dark_mac.qss $FRACTORIUM_FINAL_ROOT
cp lightdark.qss $FRACTORIUM_FINAL_ROOT
cp uranium.qss $FRACTORIUM_FINAL_ROOT
cp flam3-palettes.xml $FRACTORIUM_FINAL_ROOT
cp *.gradient $FRACTORIUM_FINAL_ROOT
cp *.ugr $FRACTORIUM_FINAL_ROOT
cp fractorium-sh $FRACTORIUM_FINAL_ROOT
cp Info.plist $FRACTORIUM_FINAL_ROOT/../
cp -r Bench $FRACTORIUM_FINAL_ROOT
cp -r examples $FRACTORIUM_FINAL_ROOT

cd $FRACTORIUM_RELEASE_ROOT

mv fractorium.app Fractorium.app

# 6. Apply ad-hoc code signature to prevent "Code Signature Invalid" crashes on macOS
codesign --force --deep --sign - Fractorium.app

hdiutil create -format UDBZ -quiet -srcfolder Fractorium.app Fractorium.dmg