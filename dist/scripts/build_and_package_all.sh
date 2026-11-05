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

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR= $"$ROOT_DIR/Data"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/Bin/release}"
APPDIR="${APPDIR:-$BUILD_DIR/Fractorium.AppDir}"
RPMAPPDIR="${RPMAPPDIR:-$BUILD_DIR/Fractorium.RpmAppDir}"
APPIMAGE_OUT="${APPIMAGE_OUT:-$BUILD_DIR/Fractorium-x86_64.AppImage}"
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
export LD_LIBRARY_PATH="${QT_PREFIX}/lib:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${QT_PREFIX}/plugins:${QT_PLUGIN_PATH:-}"

echo "== Release build starting =="
echo "Root dir:          $ROOT_DIR"
echo "Build dir:         $BUILD_DIR"
echo "App dir:           $APPDIR"
echo "AppImage out:      $APPIMAGE_OUT"
echo "Extra libs:        $EXTRA_LIBS"
echo "Qt prefix:         $QT_PREFIX"
echo "qmake:             $QMAKE_BIN"
echo "linux deploy qt:   $LINUXDEPLOYQT_BIN"
echo "AppImage tool bin: $APPIMAGETOOL_BIN"
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

if [[ ! -f "$ROOT_DIR/Fractorium.pro" ]]; then
  echo "Error: Fractorium.pro not found in $ROOT_DIR"
  exit 1
fi

mkdir -p "$BUILD_DIR" "$APPDIR/usr/bin" "$APPDIR/usr/lib" \
  "$APPDIR/usr/share/applications" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps"

# ----------------------------------------------------------------------
# Build against custom Qt
# ----------------------------------------------------------------------
echo
echo "== Building with custom Qt =="
"$QMAKE_BIN" "$ROOT_DIR/Fractorium.pro" -o "$BUILD_DIR/Makefile" -r -spec linux-g++-64 CONFIG+="release"
make -C "$BUILD_DIR" -j"$(nproc)" release

# ----------------------------------------------------------------------
# Prepare AppDir
# ----------------------------------------------------------------------
echo
echo "== Preparing AppDir =="
cp -f "$BUILD_DIR"/fractorium "$APPDIR/usr/bin/fractorium"
cp -f "$BUILD_DIR"/ember* "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$BUILD_DIR"/lib* "$APPDIR/usr/bin/" 2>/dev/null || true

# Extra runtime data required by the app
cp -f "$ROOT_DIR/Data"/*.qss "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/*.gradient "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/*.ugr "$APPDIR/usr/bin/" 2>/dev/null || true
cp -f "$ROOT_DIR/Data"/flam3-palettes.xml "$APPDIR/usr/bin/" 2>/dev/null || true
cp -a "$ROOT_DIR/Data/examples" "$APPDIR/usr/bin/" 2>/dev/null || true

# Bundle non-Qt runtime libraries required by the app.
# These were explicitly copied in the legacy build script because they are
# not always detected/packed reliably by linuxdeployqt for this project.
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

install -m 0644 "$ROOT_DIR/dist/misc/fractorium.desktop" \
  "$APPDIR/usr/share/applications/fractorium.desktop"

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
# Bundle Qt runtime and plugins into AppImage
# ----------------------------------------------------------------------
echo
echo "== Bundling Qt runtime =="
"$LINUXDEPLOYQT_BIN" \
  "$APPDIR/usr/bin/fractorium" \
  -appimage \
  -bundle-non-qt-libs \
  -qmake="$QMAKE_BIN" \
  -verbose=2 \
  -desktop_file="$APPDIR/usr/share/applications/fractorium.desktop"

# ----------------------------------------------------------------------
# Create the final AppImage
# ----------------------------------------------------------------------
echo
echo "== Creating AppImage =="
"$APPIMAGETOOL_BIN" "$APPDIR" "$APPIMAGE_OUT"

echo
echo "AppImage ready:"
echo "  $APPIMAGE_OUT"

# ----------------------------------------------------------------------
# Write/refresh Debian packaging metadata
# ----------------------------------------------------------------------
echo
echo "== Writing Debian package metadata =="
mkdir -p "$ROOT_DIR/debian"

cat > "$ROOT_DIR/debian/control" <<EOF
Source: fractorium
Section: graphics
Priority: optional
Maintainer: Matt Feemster <matt.feemster@gmail.com>
Build-Depends: debhelper-compat (= 13),
               g++,
               libgl1-mesa-dev,
               libglm-dev,
               libjpeg-dev,
               libpng-dev,
               libxml2-dev,
               libopenexr-dev,
               ocl-icd-opencl-dev,
               opencl-headers,
               qt6-base-dev,
               qmake6
Standards-Version: 4.7.0
Homepage: https://fractorium.com/
Rules-Requires-Root: no

Package: fractorium
Architecture: any
Depends: \${misc:Depends}, \${shlibs:Depends}
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

#Do not programmatically generate the debian/changelog file.
#Instead, it must be manually edited.

cat > "$ROOT_DIR/debian/rules" <<'EOF'
#!/usr/bin/make -f

export DEB_BUILD_MAINT_OPTIONS = hardening=+all
export DEB_CXXFLAGS_MAINT_APPEND = -Wall -Wextra

QT_PREFIX ?= /home/$(USER)/Dev/Qt/6.11.2/gcc_64
QMAKE_BIN ?= $(QT_PREFIX)/bin/qmake6

export PATH := $(QT_PREFIX)/bin:$(PATH)
export LD_LIBRARY_PATH := $(QT_PREFIX)/lib:$(LD_LIBRARY_PATH)

%:
	dh $@

override_dh_auto_configure:
	dh_auto_configure -- \
		QMAKE="$(QMAKE_BIN)" \
		PREFIX=/usr

override_dh_auto_build:
	$(MAKE)

override_dh_auto_install:
	$(MAKE) install INSTALL_ROOT=$(CURDIR)/debian/fractorium

	# Install the AppImage into the package-managed filesystem.
	install -d $(CURDIR)/debian/fractorium/usr/lib/fractorium
	install -m 0755 $(CURDIR)/Fractorium-x86_64.AppImage \
		$(CURDIR)/debian/fractorium/usr/lib/fractorium/Fractorium-x86_64.AppImage
EOF
chmod +x "$ROOT_DIR/debian/rules"

mkdir -p "$ROOT_DIR/debian/source"
cat > "$ROOT_DIR/debian/source/format" <<'EOF'
3.0 (quilt)
EOF

cat > "$ROOT_DIR/debian/fractorium.links" <<'EOF'
usr/lib/fractorium/Fractorium-x86_64.AppImage usr/bin/fractorium
usr/lib/fractorium/Fractorium-x86_64.AppImage usr/bin/emberrender
usr/lib/fractorium/Fractorium-x86_64.AppImage usr/bin/embergenome
usr/lib/fractorium/Fractorium-x86_64.AppImage usr/bin/emberanimate
EOF

#Benchmarking files only get used with the .deb and .rpm, not with the standalone AppImage
cp -f "$DATA_DIR/Bench/"* $APPDIR/usr/share/fractorium/bench

# ----------------------------------------------------------------------
# Build the Debian package if toolchain is available
# ----------------------------------------------------------------------
echo
echo "== Building Debian package =="
if command -v dpkg-buildpackage >/dev/null 2>&1; then
  cd "$ROOT_DIR"
  dpkg-buildpackage -us -uc -b
  echo
  echo "Debian package build complete."
  echo "Output is in the parent directory of $ROOT_DIR"
else
  echo "dpkg-buildpackage not found; Debian package was not built."
  echo "Debian metadata has been written to $ROOT_DIR/debian"
fi

# ----------------------------------------------------------------------
# Build the RPM package (legacy flow retained for compatibility)
# ----------------------------------------------------------------------
echo
echo "== Building RPM package =="
mkdir -p "$BUILD_DIR/rpmbuild/BUILD" \
         "$BUILD_DIR/rpmbuild/BUILDROOT" \
         "$BUILD_DIR/rpmbuild/RPMS" \
         "$BUILD_DIR/rpmbuild/SOURCES" \
         "$BUILD_DIR/rpmbuild/SPECS" \
         "$BUILD_DIR/rpmbuild/SRPMS" \
         "$BUILD_DIR/rpmbuild/tmp"

# Use the legacy RPM spec from the Data folder
cp -f "$ROOT_DIR/dist/rpm/Fractorium.spec" "$BUILD_DIR/rpmbuild/SPECS/Fractorium.spec"

# Create the filesystem tree expected by the legacy RPM process
rm -rf "$RPMAPPDIR"
mkdir -p "$RPMAPPDIR/DEBIAN" \
         "$RPMAPPDIR/usr/bin" \
         "$RPMAPPDIR/usr/share/applications" \
         "$RPMAPPDIR/usr/share/fractorium/bench"

# Copy the built AppImage into the package staging dir
cp -f "$APPIMAGE_OUT" "$RPMAPPDIR/usr/bin/Fractorium-x86_64.AppImage"

# Install desktop file and icon if needed
cp -f "$ROOT_DIR/dist/misc/fractorium.desktop" \
      "$RPMAPPDIR/usr/share/applications/fractorium.desktop"

cp -f "$ROOT_DIR/Source/Fractorium/Icons/Fractorium.png" \
      "$RPMAPPDIR/usr/share/fractorium/fractorium.png"

cp -f "$ROOT_DIR/debian/control" "$RPMAPPDIR/DEBIAN/control"

# Copy benchmark assets
#Benchmarking files only get used with the .deb and .rpm, not with the standalone AppImage
cp -f "$DATA_DIR/Bench/"* "$RPMAPPDIR/usr/share/fractorium/bench/" 2>/dev/null || true

# Create CLI symlinks the same way as the old script
pushd "$RPMAPPDIR/usr/bin" >/dev/null
ln -sf Fractorium-x86_64.AppImage fractorium
ln -sf Fractorium-x86_64.AppImage emberrender
ln -sf Fractorium-x86_64.AppImage embergenome
ln -sf Fractorium-x86_64.AppImage emberanimate
popd >/dev/null

# Copy the staged package into rpmbuild's BUILDROOT
cp -a "$RPMAPPDIR/usr" "$BUILD_DIR/rpmbuild/BUILDROOT/"

# Build the RPM
rpmbuild -v -bb "$BUILD_DIR/rpmbuild/SPECS/Fractorium.spec"

# Move any built RPMs into the build dir
find "$BUILD_DIR/rpmbuild/RPMS" -type f -name "*.rpm" -exec cp -f {} "$BUILD_DIR/" \;

# Rename to match the legacy naming style if desired
for rpm_file in "$BUILD_DIR"/*.rpm; do
  if [[ "$rpm_file" == *"Fractorium-"* ]]; then
    :
  else
    # Keep the spec-generated name if it matches
    :
  fi
done

# Optional rename to a predictable release name:
#Need to see how this will come out.
#mv "$BUILD_DIR/Fractorium-<generated>.rpm" "$BUILD_DIR/Fractorium-${APP_VERSION}.x86_64.rpm"
mv "$BUILD_DIR/Fractorium-${APP_VERSION}-1.x86_64.rpm" "$BUILD_DIR/Fractorium-${APP_VERSION}.x86_64.rpm"

echo "RPM package build complete."

echo
echo "== Release pipeline finished =="
echo "AppImage:  $APPIMAGE_OUT"
echo "Debian metadata: $ROOT_DIR/debian"
echo
echo "Note: the AppImage still preserves the CLI symlink behavior:"
echo "  fractorium"
echo "  emberrender"
echo "  embergenome"
echo "  emberanimate"