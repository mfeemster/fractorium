#!/bin/bash
#Archival, no longer supported

USAGE="`basename $0`
Run this script from the archive folder.

Without options, the default is to build a signed source package for uploading
to a Launchpad PPA.

Options:

-h  --help

--binary-only
--source-only
--source-and-binary
--signed
--unsigned"

cd ..
PROJECT=fractorium

changelogVersion=$(cat ./debian/changelog | head -n 1 | sed 's/^'$PROJECT' (\([^-]\+\)-.*/\1/')
emberVersion=$(grep '#define EMBER_VERSION' ./Source/Ember/EmberDefines.h | sed 's/^.*EMBER_VERSION "\([^"]\+\)".*/\1/')

a=$(echo -n "$changelogVersion" | sed 's/[a-z]//g')
b=$(echo -n "$emberVersion" | sed 's/[a-z]//g')

if [[ "$a" != "$b" ]]; then
	echo "Error: Different version numbers were found. Please update the correct file,
the version numbers should agree up to the digits in:

$changelogVersion
$emberVersion

./debian/changelog            : $a
./Source/Ember/EmberDefines.h : $b"
	exit 2
fi

VERSION=$changelogVersion
PROJECT_ROOT=$PWD
PPA_DIR="$HOME/PPA/$PROJECT-$VERSION"
TAR_NAME="$PROJECT-$VERSION.tar.gz"

if [ ! -d '.git' -o ! -f 'main.pro' ]; then
	echo "Run `basename $0` from the project root."
	exit 2
fi

# Set defaults.
OPT_BUILD_BINARY=0
OPT_BUILD_SOURCE=1
OPT_SIGNED=1

while [ $# -gt 0 ]; do
	case "$1" in
		--binary-only)       OPT_BUILD_SOURCE=0
							 OPT_BUILD_BINARY=1
							 ;;
		--source-only)       OPT_BUILD_SOURCE=1
							 OPT_BUILD_BINARY=0
							 ;;
		--source-and-binary) OPT_BUILD_SOURCE=1
							 OPT_BUILD_BINARY=1
							 ;;
		--signed)            OPT_SIGNED=1
							 ;;
		--unsigned)          OPT_SIGNED=0
							 ;;
		-h|--help) echo "$USAGE"
				   exit 0;;
	esac
	shift
done

tarversion=$(tar --version | head -1 | sed -e 's/tar (GNU tar) \+\([0-9\.]\+\)$/\1/; s/[^0-9]//g; s/\(.\{3\}\).*/\1/;')

if [[ "$tarversion" -lt "128" ]]; then
	echo "Tar >= 1.28 is required. Download the .deb from https://launchpad.net/ubuntu/+source/tar/ and install manually."
	exit 2
fi

if [ ! -d "$PPA_DIR" ]; then
	mkdir -p "$PPA_DIR"
else
	echo -n "PPA work folder already exists: $PPA_DIR
Move this folder aside or remove it.
"
	exit 2
fi

# tar 1.28 required for --exclude-vcs-ignores

# NOTE: ./Data/flam3-palettes.xml has to be explicitly included. The *.xml rule
# would exclude it, and tar doesn't handle "!" rules.

tar --exclude='package-linux.sh' \
	--exclude='debian' \
	--exclude='Bin' \
	--exclude-vcs \
	--exclude-vcs-ignores \
	--exclude-backups \
	-czf "$PPA_DIR/$TAR_NAME" \
	./Data/flam3-palettes.xml \
	./Data/boxtail_pack_02.gradient \
	./Data/boxtail_pack_03_triangle.gradient \
	./Data/boxtail_pack_04_mineshack.gradient \
	./Data/fardareismai_pack_01_variety_number_128.gradient \
	./Data/fardareismai_pack_02_b_sides.gradient \
	./Data/fardareismai_pack_03_old_and_new.gradient \
	./Data/fardareismai_pack_04_hoard.gradient \
	./Data/fractaldesire_pack_01.gradient \
	./Data/rce_ordinary_pack_01_colornation.gradient \
	./Data/tatasz_pack_01.gradient \
	./Data/tatasz_pack_02_colder.gradient \
	./Data/tatasz_pack_02_dark.gradient \
	./Data/tatasz_pack_02_warmer.gradient \
	./Data/tatasz_pack_03.gradient \
	./Data/tatasz_pack_04.gradient \
	./Data/tatasz_pack_05.gradient \
	./Data/tatasz_pack_06.gradient \
	./Data/tatasz_pack_07.gradient \
	./Data/Amphibole_Supergroup.ugr \
	./Data/Apatite_Supergroup.ugr \
	./Data/Feldspar_Group.ugr \
	./Data/Mica_Group.ugr \
	./Data/Quartz_Varieties.ugr \
	./Data/dark_linux.qss \
	./Data/lightdark.qss \
	./Data/uranium.qss \
	./Data/examples/b33rheart_examples.flame \
	./Data/examples/b33rheart_sierpinski.flame \
	./Data/examples/c-91_examples.flame \
	./Data/examples/pillemaster_hexagonal_tilings.flame \
	./Data/examples/plangkye_examples.flame \
	./Data/examples/tatasz_examples.flame \
	./Data/examples/tatasz_substitution.flame \
	./Data/examples/triptychaos_examples.flame \
	./Data/examples/tyrantwave_base_forms.flame \
	.

[ $? -ne 0 ] && echo "Tar command failed." && exit 2

cd "$PPA_DIR"
bzr dh_make $PROJECT $VERSION $TAR_NAME

[ $? -ne 0 ] && echo "bzr dh_make command failed." && exit 2

rm "$PPA_DIR/fractorium/debian" -r
cp -R "$PROJECT_ROOT/debian" "$PPA_DIR/fractorium"

cd "$PPA_DIR/fractorium" &&\
	bzr add . &&\
	bzr commit -m "Debian package $VERSION"

[ $? -ne 0 ] && echo "bzr command failed." && exit 2

# Build a source package.

# Launchpad only needs a signed source package. It will build its own binary on
# the servers.

if [ $OPT_BUILD_SOURCE -eq 1 ]; then
	if [ $OPT_SIGNED -eq 1 ]; then
		bzr builddeb -- -S
	else
		bzr builddeb -- -S -us -uc
	fi
fi

[ $? -ne 0 ] && echo "bzr builddeb for source package failed." && exit 2

# Build an binary package.

if [ $OPT_BUILD_BINARY -eq 1 ]; then
	if [ $OPT_SIGNED -eq 1 ]; then
		bzr builddeb -- -b
	else
		bzr builddeb -- -b -us -uc
	fi
fi

[ $? -ne 0 ] && echo "bzr builddeb for source package failed." && exit 2

cd archive