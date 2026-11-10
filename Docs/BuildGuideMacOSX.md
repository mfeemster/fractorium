# Build Guide for Mac OS/X

Install Xcode from the App Store

Install Qt 6.11.2

Install [homebrew](http://brew.sh/).

Install Command Line Tools for Xcode

Clone the repository:

```
git clone https://github.com/mfeemster/fractorium.git
```

Install the dependencies:

```
brew install glm jpeg libpng glib openexr@2
```

Add the Qt `bin` folder to `PATH` to make `qmake` available. In
`~/.bash_profile` or `~/.bashrc`:

```
PATH=/usr/local/opt/qt6/bin:$PATH
export PATH
```

Building:

```
cd fractorium
qmake CONFIG+=release
make
```
Or open main.pro in Qt Creator, select release and build all.

Creating the app bundle:

```
cd archive
./build.sh
```

Running the binary from the release folder:

```
cd ..
cd Bin/release
./fractorium
```

Installing:

```
Open Fractorium.dmg and copy Fractorium.app to /Applications.
```
