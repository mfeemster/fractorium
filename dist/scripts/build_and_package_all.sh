#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------------------------------------
# Fractorium Release Build
#
# Usage:
#   QT_PREFIX=/home/$USER/Dev/Qt/6.11.2/gcc_64 ./release_build.sh
#
# This script:
#   1) builds against a custom Qt install
#   2) creates the AppImage
#   3) writes/updates Debian packaging metadata
#   4) builds the .deb package
#
# The AppImage preserves the old CLI symlink behavior:
#   fractorium
#   emberrender
#   embergenome
#   emberanimate
# ----------------------------------------------------------------------

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && cd ../.. && pwd)"
DATA_DIR="${ROOT_DIR}/Data"
DIST_DIR="${ROOT_DIR}/dist"
PROJECTS_DIR="${ROOT_DIR}/Builds/QtCreator"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/Bin/release}"
APPDIR="${APPDIR:-$BUILD_DIR/Fractorium.AppDir}"
DEBDIR="${DEBDIR:-$BUILD_DIR/Fractorium.DebDir}"
RPMDIR="${RPMAPPDIR:-$BUILD_DIR/Fractorium.RpmDir}"
APPIMAGE_OUT="${APPIMAGE_OUT:-$ROOT_DIR/Fractorium-x86_64.AppImage}"
APP_VERSION="${APP_VERSION:-26.26.10.1}"
DEBIAN_REV="${DEBIAN_REV:-1}"
DISTRO="${DISTRO:-noble}"
URGENCY="${URGENCY:-medium}"

QT_PREFIX="${QT_PREFIX:-$HOME/Dev/Qt/6.11.2/gcc_64}"
QMAKE_BIN="${QMAKE_BIN:-$QT_PREFIX/bin/qmake6}"
LINUXDEPLOYQT_BIN="${LINUXDEPLOYQT_BIN:-$HOME/Dev/DeployTools/linuxdeploy-plugin-qt-x86_64.AppImage}"
APPIMAGETOOL_BIN="${APPIMAGETOOL_BIN:-$HOME/Dev/DeployTools/linuxdeploy-x86_64.AppImage}"
EXTRA_LIBS="${EXTRA_LIBS:-/usr/lib/x86_64-linux-gnu}"

export PATH="${QT_PREFIX}/bin:${PATH}"
#export LD_LIBRARY_PATH="${QT_PREFIX}/lib:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${QT_PREFIX}/plugins:${QT_PLUGIN_PATH:-}"

echo "== Release build starting =="
echo "Root dir:          $ROOT_DIR"
echo "Build dir:         $BUILD_DIR"
echo "Projects dir:      $PROJECTS_DIR"
echo "Dist dir:          $DIST_DIR"
echo "App dir:           $APPDIR"
echo "AppImage out:      $APPIMAGE_OUT"
echo "Extra libs:        $EXTRA_LIBS"
echo "Qt prefix:         $QT_PREFIX"
echo "qmake:             $QMAKE_BIN"
echo "linux deploy qt:   $LINUXDEPLOYQT_BIN"
echo "AppImage deploy:   $APPIMAGETOOL_BIN"
echo "LD library path:   $LD_LIBRARY_PATH"
echo "Qt plugin path:    $QT_PLUGIN_PATH"
echo "App version:       $APP_VERSION"
echo "Debian rev:        $DEBIAN_REV"
echo "Target distro:     $DISTRO"
echo "Urgency:           $URGENCY"

# ----------------------------------------------------------------------
# Sanity checks
# ----------------------------------------------------------------------

if [[ ! -x "$QMAKE_BIN" ]]; then
  echo "Error: qmake not found at $QMAKE_BIN"
  exit 1
fi

if [[ ! -e "$LINUXDEPLOYQT_BIN" ]]; then
  echo "Error: linuxdeployqt not found at $LINUXDEPLOYQT_BIN"
  exit 1
fi

if [[ ! -e "$APPIMAGETOOL_BIN" ]]; then
  echo "Error: appimagetool not found at $APPIMAGETOOL_BIN"
  exit 1
fi

if [[ ! -f "$PROJECTS_DIR/defaults.pri" ]]; then
  echo "Error: Fractorium.pro not found in $PROJECTS_DIR/defaults.pri"
  exit 1
fi

if [[ ! -f "$PROJECTS_DIR/Fractorium/Fractorium.pro" ]]; then
  echo "Error: Fractorium.pro not found in $PROJECTS_DIR/Fractorium/Fractorium.pro"
  exit 1
fi

#rm -rf "$BUILD_DIR" ; Do this for a clean rebuild.
rm -rf "$APPDIR"
mkdir -p "$BUILD_DIR" "$APPDIR/usr/bin" "$APPDIR/usr/lib" \
  "$APPDIR/usr/share/applications" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# ----------------------------------------------------------------------
# Build against custom Qt
# ----------------------------------------------------------------------
echo
echo "== Building with custom Qt =="
echo "== Running qmake =="
"$QMAKE_BIN" "$ROOT_DIR/main.pro" -r -spec linux-g++-64 CONFIG+="release"
echo "== Running make =="
make

# ----------------------------------------------------------------------
# Prepare AppDir
# ----------------------------------------------------------------------
echo
echo "== Copying Fractorium binaries to AppDir =="
cp -f "$BUILD_DIR"/fractorium "$APPDIR/usr/bin/fractorium"
cp -f "$BUILD_DIR"/ember* "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$BUILD_DIR"/lib* "$APPDIR/usr/bin/" 2>/dev/null || true

# Extra runtime data required by the app
echo "== Copying palettes, stylesheets and examples to AppDir =="
cp -f "$ROOT_DIR/Data"/*.qss "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/*.gradient "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/*.ugr "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/flam3-palettes.xml "$APPDIR/usr/bin/" 2>/dev/null || true
cp -a "$ROOT_DIR/Data/examples" "$APPDIR/usr/bin/" 2>/dev/null || true

# Bundle non-Qt runtime libraries required by the app.
# These were explicitly copied in the legacy build script because they are
# not always detected/packed reliably by linuxdeployqt for this project.
echo "== Copying third party libraries AppDir =="
for lib in \
  libIex.so \
  libIlmThread.so \
  libImath.so \
  libjpeg.so.8 \
  libpng16.so.16 \
  libOpenCL.so.1 \
  libxcb-cursor.so.0 \
  libz.so
do
  if [[ -f "$EXTRA_LIBS/$lib" ]]; then
    cp -f "$EXTRA_LIBS/$lib" "$APPDIR/usr/lib/"
  fi
done

# Desktop file and icon
cp -f "$ROOT_DIR/Source/Fractorium/Icons/Fractorium.png" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps/fractorium.png"

cat > "$APPDIR/usr/share/applications/fractorium.desktop" <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=Fractorium
GenericName=Fractal flame editor
Comment=Fractal flame editor with GPU support
Exec=fractorium
TryExec=fractorium
Icon=fractorium
Terminal=false
Categories=Graphics
StartupNotify=true
EOF

#install -m 0644 "$ROOT_DIR/dist/misc/fractorium.desktop" \
#  "$APPDIR/usr/share/applications/fractorium.desktop"

# ----------------------------------------------------------------------
# AppRun must preserve the symlink behavior
# ----------------------------------------------------------------------
cat > "$APPDIR/AppRun" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

APPDIR="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${APPDIR}/usr/lib/plugins:${QT_PLUGIN_PATH:-}"

CMD_NAME="$(basename "$0")"

case "$CMD_NAME" in
  fractorium)
    exec "${APPDIR}/usr/bin/fractorium" "$@"
    ;;
  emberrender)
    exec "${APPDIR}/usr/bin/emberrender" "$@"
    ;;
  embergenome)
    exec "${APPDIR}/usr/bin/embergenome" "$@"
    ;;
  emberanimate)
    exec "${APPDIR}/usr/bin/emberanimate" "$@"
    ;;
  *)
    exec "${APPDIR}/usr/bin/fractorium" "$@"
    ;;
esac
EOF
chmod +x "$APPDIR/AppRun"

# ----------------------------------------------------------------------
# Create the final AppImage
# ----------------------------------------------------------------------
echo
echo "== Creating AppImage =="
$APPIMAGETOOL_BIN --appdir=$APPDIR --output=appimage --executable=$APPDIR/usr/bin/fractorium --desktop-file=$APPDIR/usr/share/applications/fractorium.desktop --plugin=qt
chmod +x "$APPIMAGE_OUT"
cp -f Fractorium-x86_64.AppImage $DIST_DIR/Fractorium-$APP_VERSION.x86_64.AppImage

echo
echo "AppImage ready:"
echo "  $APPIMAGE_OUT"

# ----------------------------------------------------------------------
# Write/refresh Debian packaging metadata
# ----------------------------------------------------------------------
echo
echo "== Writing Debian package metadata =="

rm -rf "$DEBDIR"
mkdir -p "$DEBDIR"
mkdir -p "$DEBDIR/DEBIAN"
mkdir -p "$DEBDIR/debian"
mkdir -p "$DEBDIR/debian/source"
mkdir -p "$DEBDIR/usr/bin"
mkdir -p "$DEBDIR/usr/share/applications"
mkdir -p "$DEBDIR/usr/share/fractorium/bench"

mv Fractorium-x86_64.AppImage "$DEBDIR/usr/bin"
cd $DEBDIR/usr/bin

create_symlinks()
{
   ln -s Fractorium-x86_64.AppImage fractorium
   ln -s Fractorium-x86_64.AppImage emberrender
   ln -s Fractorium-x86_64.AppImage embergenome
   ln -s Fractorium-x86_64.AppImage emberanimate
}

create_symlinks
cd $BUILD_DIR

#Note the capital DEBIAN here
cat > "$DEBDIR/DEBIAN/control" <<EOF
Package: fractorium
Source: fractorium
Version: $APP_VERSION
Architecture: amd64
Section: graphics
Priority: optional
Maintainer: Matt Feemster <mfeemster@protonmail.com>
Build-Depends: debhelper-compat (= 13)
Standards-Version: 4.7.0
Homepage: https://fractorium.com
Vcs-Git: https://github.com/mfeemster/fractorium.git
Vcs-Browser: https://github.com/mfeemster/fractorium
Rules-Requires-Root: no
Description: fractal flame editor with GPU support
 Fractorium
 .
 A fractal flame editor with GPU support.
 .
 http://fractorium.com/
 .
 The project is maintained on github:
 .
 https://github.com/mfeemster/fractorium
 .
 News and updates at the above or on Matt Feemster's DeviantArt page:
 .
 https://www.deviantart.com/mfeemster
EOF

cat > "$DEBDIR/debian/copyright" <<'EOF'
Format: http://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: fractorium
Source: https://github.com/mfeemster/fractorium

Files: *
Copyright: 2026 Matt Feemster <mfeemster@protonmail.com>
License: GPL-2+
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 2 can be found in "/usr/share/common-licenses/GPL-2".
EOF

#Do not programmatically generate the debian/changelog file.
#Instead, it must be manually edited.
cp $DIST_DIR/debian/changelog $DEBDIR/debian/changelog

cat > "$DEBDIR/debian/rules" <<'EOF'
#!/usr/bin/make -f
%:
	dh $@
EOF
chmod +x "$DEBDIR/debian/rules"

cat > "$DEBDIR/debian/source/format" <<'EOF'
3.0 (quilt)
EOF

#Reuse the desktop file created for the AppImage
cp $APPDIR/usr/share/applications/fractorium.desktop $DEBDIR/usr/share/applications/fractorium.desktop
cp $ROOT_DIR/Source/Fractorium/Icons/Fractorium.png $DEBDIR/usr/share/fractorium/fractorium.png

#Benchmarking files only get used with the .deb and .rpm, not with the standalone AppImage
cp -f $DATA_DIR/Bench/* $DEBDIR/usr/share/fractorium/bench

# ----------------------------------------------------------------------
# Build the Debian package if toolchain is available
# Explicitly specify that the debian folder is in a subfolder named dist
# ----------------------------------------------------------------------
echo
echo "== Building Debian package =="
cd "$BUILD_DIR"
#cd "$DEBDIR"

if command -v dpkg-buildpackage >/dev/null 2>&1; then
#  dpkg-buildpackage -b
  dpkg -b Fractorium.DebDir
  echo
  echo "Debian package build complete."
  echo "Moving package to $DIST_DIR"
  mv -f Fractorium.DebDir.deb $DIST_DIR/Fractorium-$APP_VERSION.x86_64.deb
  echo "Output is in $DIST_DIR/Fractorium-$APP_VERSION.x86_64.deb"
else
  echo "dpkg-buildpackage not found; Debian package was not built. Packaging process exiting."
  exit 1
fi

cd "$ROOT_DIR"

# ----------------------------------------------------------------------
# Build the RPM package (legacy flow retained for compatibility)
# ----------------------------------------------------------------------
echo
echo "== Building RPM package =="
rm -rf "$RPMDIR"
mkdir -p "$RPMDIR/BUILD" \
         "$RPMDIR/BUILDROOT" \
         "$RPMDIR/RPMS" \
         "$RPMDIR/SOURCES" \
         "$RPMDIR/SPECS" \
         "$RPMDIR/SRPMS" \
         "$RPMDIR/tmp"

cat > "$RPMDIR/SPECS/Fractorium.spec" <<EOF
%define _topdir       %(pwd)
%define buildroot     $RPMDIR/BUILDROOT

Name:           Fractorium
Version:        $APP_VERSION
Release:        1
Summary:        A fractal flame editor with GPU support

Group:          Graphics
BuildArch:      x86_64
License:        GPL
URL:            https://github.com/mfeemster/fractorium
BuildRoot:      %{buildroot}

%description
Fractorium

A fractal flame editor with GPU support.

http://fractorium.com/

The project is maintained on github:

https://github.com/mfeemster/fractorium

News and updates at the above or on Matt Feemster's DeviantArt page:

https://www.deviantart.com/mfeemster


%files
%attr(0755, root, root) "/usr/bin/Fractorium-x86_64.AppImage"
/usr/bin/emberanimate
/usr/bin/embergenome
/usr/bin/emberrender
/usr/bin/fractorium
%attr(0644, root, root) "/usr/share/applications/fractorium.desktop"
%attr(0644, root, root) "/usr/share/fractorium/fractorium.png"

#bench files
%attr(0644, root, root) "/usr/share/fractorium/bench/EmberBench.ps1"
%attr(0644, root, root) "/usr/share/fractorium/bench/golubaja_rippingfrominside_complexcode.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/mfeemster_basicmemory.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/tatasz_springcrown_manysimplexforms.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/tyrantwave_flippeddisc_normal.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/zy0rg_six_bigcomplexcode.flame"

%changelog
EOF

#Instead of manually recreating the same folder structure, just copy the one we made above for the deb
cp -r $DEBDIR/usr $RPMDIR/BUILDROOT

cd $RPMDIR
# Build the RPM
rpmbuild -v -bb "$RPMDIR/SPECS/Fractorium.spec"

# Rename to a predictable release name
mv "$RPMDIR/RPMS/x86_64/Fractorium-${APP_VERSION}-1.x86_64.rpm" "$DIST_DIR/Fractorium-${APP_VERSION}.x86_64.rpm"

echo "RPM package build complete."

echo
echo "== Release pipeline finished =="
echo "AppImage:  $DIST_DIR/Fractorium-${APP_VERSION}.x86_64.AppImage"
echo "Deb:       $DIST_DIR/Fractorium-${APP_VERSION}.x86_64.deb"
echo "RPM:       $DIST_DIR/Fractorium-${APP_VERSION}.x86_64.rpm"

cd $ROOT_DIR
