#!/bin/bash

echo "Creating the AppImage."

BUILD_PATH=$PWD
FRACTORIUM_RELEASE_ROOT=$PWD/../Bin/release
DATA_PATH=$BUILD_PATH/../Data
ICON_PATH=$BUILD_PATH/../Source/Fractorium/Icons
APP_DIR=$PWD/../Bin/Fractorium.AppDir
FRACTORIUM_PACKAGE=$BUILD_PATH/../Bin/Fractorium
FRACTORIUM_RPM_PACKAGE=$BUILD_PATH/../Bin/rpmbuild
EXTRA_LIBS=/usr/lib/x86_64-linux-gnu

# replace 6.5.1 with your QT version, and ensure the installation path is the same
QT_PATH=/home/$USER/Dev/Qt/6.5.1/gcc_64/bin

LINUX_DEPLOY_QT=/home/$USER/Dev/linuxdeploy-plugin-qt-x86_64.AppImage
APP_IMAGE_TOOL=/home/$USER/Dev/linuxdeploy-x86_64.AppImage

#######################
#simple check

if [ ! -d "$FRACTORIUM_RELEASE_ROOT" ]; then
   echo "release folder not found. Please, build the project."
   exit 2
fi


if [ -d "$APP_DIR" ]; then
   echo "Fractorium.AppDir folder already exists at $APP_DIR and will be removed now."
   rm -rf $APP_DIR
fi

if [ -d "$FRACTORIUM_PACKAGE" ]; then
   echo "Fractorium folder already exists in at $FRACTORIUM_PACKAGE and will be removed now"
   rm -rf $FRACTORIUM_PACKAGE
fi

if [ -d "$FRACTORIUM_RPM_PACKAGE" ]; then
   echo "rpmbuild folder already exists in Bin: $FRACTORIUM_RPM_PACKAGE and will be removed now"
   rm -rf $FRACTORIUM_RPM_PACKAGE
fi

if [ ! -d "$QT_PATH" ]; then
   echo "QT folder $QT_PATH not found. Please, change QT_PATH."
   exit 2
fi

check_apps()
{
  FILE=$1
  echo $FILE
  if [ ! -e "$FILE" ] ; then
    echo "Application not found: " $FILE
    exit 0
  fi
}

check_apps "$LINUX_DEPLOY_QT"
check_apps "$APP_IMAGE_TOOL"

#######################

export PATH=$QT_PATH:$PATH

mkdir -p $APP_DIR

cd $APP_DIR

FRACTORIUM_BIN=usr/bin
FRACTORIUM_LIB=usr/lib
FRACTORIUM_SHR=usr/share/applications
FRACTORIUM_ICO=usr/share/icons/hicolor/256x256/apps

mkdir -p $FRACTORIUM_BIN
mkdir -p $FRACTORIUM_LIB
mkdir -p $FRACTORIUM_SHR
mkdir -p $FRACTORIUM_ICO

cp $FRACTORIUM_RELEASE_ROOT/ember* $FRACTORIUM_BIN
cp $FRACTORIUM_RELEASE_ROOT/fractorium $FRACTORIUM_BIN
cp $FRACTORIUM_RELEASE_ROOT/lib* $FRACTORIUM_LIB

cp $EXTRA_LIBS/libIex.so $FRACTORIUM_LIB
cp $EXTRA_LIBS/libIlmThread.so $FRACTORIUM_LIB
cp $EXTRA_LIBS/libImath.so $FRACTORIUM_LIB
cp $EXTRA_LIBS/libjpeg.so.8 $FRACTORIUM_LIB
cp $EXTRA_LIBS/libpng16.so.16 $FRACTORIUM_LIB
cp $EXTRA_LIBS/libOpenCL.so.1 $FRACTORIUM_LIB
cp $EXTRA_LIBS/libxcb-cursor.so.0 $FRACTORIUM_LIB

cp $EXTRA_LIBS/libz.so $FRACTORIUM_LIB

cp $DATA_PATH/*.qss $FRACTORIUM_BIN
cp $DATA_PATH/flam3-palettes.xml $FRACTORIUM_BIN
cp $DATA_PATH/*.gradient $FRACTORIUM_BIN
cp $DATA_PATH/*.ugr $FRACTORIUM_BIN
cp -ar $DATA_PATH/examples $FRACTORIUM_BIN
cp $ICON_PATH/Fractorium.png $FRACTORIUM_ICO/fractorium.png
cp $DATA_PATH/fractorium.appimage.desktop $FRACTORIUM_SHR/fractorium.desktop

cd ../

#This is critical because otherwise it can't find libember.so and libembercl.so in the lib folder.
export LD_LIBRARY_PATH="${APP_DIR}/usr/lib:${LD_LIBRARY_PATH}"
echo "LD_LIBRARY_PATH is" $LD_LIBRARY_PATH

cp $DATA_PATH/AppRun $APP_DIR

$APP_IMAGE_TOOL --appdir $APP_DIR --output appimage --executable $APP_DIR/usr/bin/fractorium --desktop-file $APP_DIR/usr/share/applications/fractorium.desktop --plugin qt

echo ""
echo "Creating the DEB package."
echo ""

mkdir    Fractorium
mkdir -p Fractorium/DEBIAN
mkdir -p Fractorium/usr/bin
mkdir -p Fractorium/usr/share/applications
mkdir -p Fractorium/usr/share/fractorium/bench

cp Fractorium-x86_64.AppImage Fractorium/usr/bin

cp $DATA_PATH/fractorium.package.desktop Fractorium/usr/share/applications/fractorium.desktop

cp $ICON_PATH/Fractorium.png Fractorium/usr/share/fractorium/fractorium.png

cp $DATA_PATH/control.package Fractorium/DEBIAN/control

cp $DATA_PATH/Bench/* Fractorium/usr/share/fractorium/bench

#creating symbolic links
cd ./Fractorium/usr/bin

create_symlinks()
{
   ln -s Fractorium-x86_64.AppImage fractorium
   ln -s Fractorium-x86_64.AppImage emberrender
   ln -s Fractorium-x86_64.AppImage embergenome
   ln -s Fractorium-x86_64.AppImage emberanimate
}

create_symlinks

cd ../../../

dpkg --build Fractorium

echo ""
echo "Creating RPM package"
echo ""

mkdir    rpmbuild
mkdir -p rpmbuild/BUILD
mkdir -p rpmbuild/BUILDROOT
mkdir -p rpmbuild/RPMS
mkdir -p rpmbuild/SOURCES
mkdir -p rpmbuild/SPECS
mkdir -p rpmbuild/SRPMS
mkdir -p rpmbuild/tmp

cp $DATA_PATH/Fractorium.spec rpmbuild/SPECS

cd Fractorium
cp -r usr ../rpmbuild/BUILDROOT
cd ../rpmbuild

rpmbuild -v -bb SPECS/Fractorium.spec

cd ../

mv rpmbuild/RPMS/x86_64/* ./

emberVersion=$(grep '#define EMBER_VERSION' ../Source/Ember/EmberDefines.h | sed 's/^.*EMBER_VERSION "\([^"]\+\)".*/\1/')

mv Fractorium.deb Fractorium-$emberVersion.x86_64.deb
mv Fractorium-$emberVersion-1.x86_64.rpm Fractorium-$emberVersion.x86_64.rpm

echo ""
echo "Finishing AppImage"
echo ""

rm -rf Fractorium

mkdir Fractorium

cp Fractorium-x86_64.AppImage Fractorium
cd Fractorium

create_symlinks

mkdir bench

cp $DATA_PATH/Bench/* bench

cd ../

tar -czvf Fractorium-$emberVersion.x86_64.AppImage.tar.gz Fractorium
mv Fractorium-x86_64.AppImage Fractorium-$emberVersion.x86_64.AppImage

#cleaning
rm -rf Fractorium
rm -rf Fractorium.AppDir
rm -rf rpmbuild
# rm -rf Fractorium-$emberVersion.x86_64.AppImage
