# Linux Release

These are instructions for publishing a Linux deb package. It can be built locally or the PPA repository can build it. The latter is preferable.

## Summary

### Building a `.deb` locally

Update `debian/changelog` with a new log message. Update version number. Use the time from `date -R`.

Build:

```
$ ./package-linux.sh --binary-only --unsigned
```

Type `s` for single binary, `Enter`. It displays a confirm message, `Enter`.

When the build finished, find the `.deb` in `~/PPA/fractorium-VERSION`.

### Publishing to Launchpad PPA

Remember we are not building a `.deb` here, Launchpad will do that. We are just
uploading a signed source tarball.

Make sure `debian/changelog` was updated.

Build:

```
$ ./package-linux.sh
```

Type `s` for single binary, `Enter`. It displays a confirm message, `Enter`.

It asks for your GPG key passphrase to sign the source tarball.

Now upload the source tarball to Launchpad:

```
$ cd ~/PPA/fractorium-VERSION/
$ dput ppa:fractorium/ppa fractorium_VERSION-0ubuntu1_source.changes
```

An email will arrive to say if the package was accepted or rejected.

Wait for the autobuild to complete, and when the package is published, copy the
link to the `.deb` from "Package Details".

## Narrative

Starting with a fresh clone from bitbucket.org. It will not work unless it's totally fresh:

```
$ git clone https://mfeemster@bitbucket.org/mfeemster/fractorium.git
$ cd fractorium
```

Test that the package creator script:

`package-linux.sh` with no arguments builds a signed source for the Launchpad PPA.

Instead, now we want an unsigned binary `.deb`:

```
$ ./package-linux.sh --binary-only --unsigned
```

```
Error: Different version numbers were found. Please update the correct file,
the version numbers should agree up to the digits in:

a.b.c.d
w.x.y.z

./debian/changelog            : a.b.c.d
./Source/Ember/EmberDefines.h : w.x.y.z
```

Change the version number in `debian/changelog` to match the one in `EmberDefines.h`. Next, get the time in the correct format and add it to a new entry in the changelog:

```
$ date -R
Sat, 18 Jun 2016 13:12:15 +0100
```

Add a new log message at the top of `debian/changelog`, copying the last message
and changing the version number and time.

```
fractorium (w.x.y.z-0ubuntu1) xenial; urgency=low

  * release w.x.y.z

 -- Matt Feemster <matt.feemster@gmail.com>  Sat, 18 Jun 2016 13:12:15 +0100
```

Now try again:

```
$ ./package-linux.sh --binary-only --unsigned
```

You will be prompted with a question about the type of build:

```
Type of package: single binary, indep binary, multiple binary, library, kernel module, kernel patch?
 [s/i/m/l/k/n]
```

Type `s` for single binary, `Enter`.

It displays a confirmation message, `Enter`.

The build starts. It copied the files which are not ignored in `.gitignore` out
to `~/PPA/fractorium-VERSION/`, and built it there.

Verify the build finished:

```
$ cd ~/PPA/fractorium-w.x.y.z
$ ls -lh
total 7.7M
drwxrwxr-x 2 user user 4.0K Jun 18 13:31 build-area
drwxrwxr-x 8 user user 4.0K Jun 18 13:19 fractorium
-rw------- 1 user user  780 Jun 18 13:31 fractorium_w.x.y.z-0ubuntu1_amd64.changes
-rw------- 1 user user 3.5M Jun 18 13:31 fractorium_w.x.y.z-0ubuntu1_amd64.deb
-rw-rw-r-- 1 user user 2.1M Jun 18 13:18 fractorium_w.x.y.z.orig.tar.gz
-rw-rw-r-- 1 user user 2.1M Jun 18 13:18 fractorium-w.x.y.z.tar.gz
```

This `.deb` is ready to use.

(aside)

You could upload this `.deb` to the website if you don't want to bother with the
Launchpad PPA. The advantages of the PPA are

- Testing the package and build procedure
- Users will automatically get the updated version when they run their regular upgrades

People prefer using the PPA to install and remain up to date like so:

```
sudo apt-add-repository ppa:fractorium/ppa
sudo apt-get update
sudo apt-get install fractorium
```

(aside end)

Installing the `.deb`:

```
sudo dpkg -i fractorium_w.x.y.z-0ubuntu1_amd64.deb
```

Verify fractorium loads with its menu icon, and the About page has the updated
version number. Run a few random flames, drag some xforms around, and test GPU support if you have it.

Upload the source to the Launchpad PPA for the auto-build.

In the original fractorium source folder:

```
$ ./package-linux.sh
PPA work folder already exists: /home/yume/PPA/fractorium-w.x.y.z
Move this folder aside or remove it.
```

It expects to be able to create a clean folder with that name and the earlier files aren't needed to remove it:

```
$ rm -r ~/PPA/fractorium-1.0.0.0
```

Try again:

```
$ ./package-linux.sh
```

Press `s` at the type of package question.

This is quick, it only creates a source tarball to upload.

Before finishing, it asks for your GPG key passphrase to sign the source tarball.

Upload it to Launchpad:

```
$ cd ~/PPA/fractorium-w.x.y.z/
$ dput ppa:fractorium/ppa fractorium_w.x.y.z-0ubuntu1_source.changes 
```

It verifies your signature and says `Uploading to ppa (via ftp to ppa.launchpad.net)`.

Upload finished, now open your email and wait for a message from Launchpad. It
arrives, saying the package was accepted.

```
[~fractorium/ubuntu/ppa/xenial] fractorium w.x.y.z-0ubuntu1 (Accepted)
```

Open https://launchpad.net/~fractorium/+archive/ubuntu/ppa from the email and check that the build started:

```
[BUILDING]	amd64 build of fractorium w.x.y.z-0ubuntu1 in ubuntu xenial RELEASE
Build started 2 minutes ago on lgw01-09
```

This will take several minutes.

If the package had been rejected, or if the autobuild now fails, then fix the build on your machine. When you want to try again, add a new
message to the top of `debian/changelog`, with a modified version number and
time. For these minor changes it is enough to append a letter to the version
number, such as `fractorium 1.0.0.0a`. Then create a tarball again with
`package-linux.sh`, and upload to Launchpad with `dput`.

Update the project links and description in
`debian/control`. Bump the version number to `fractorium 1.0.0.0a` and update the time to `date -R`.

Publish update:

```
$ ./package-linux.sh
$ cd ~/PPA/fractorium-w.x.y.za
$ dput ppa:fractorium/ppa fractorium_w.x.y.za-0ubuntu1_source.changes
```

Email says package accepted. Building has started. Finished:

```
Successfully built on lgw01-33

Started 23 minutes ago
Finished 1 minute ago (took 22 minutes, 6.6 seconds)
```

But also:

```
Binary packages awaiting publication:

fractorium-w.x.y.za-0ubuntu1
```

It usually takes a bit more time until the `.deb` is published in the PPA repository.

Commit the changes:

```
$ git checkout -b linux-w.x.y.za
$ git add -A .
$ git commit -m "linux w.x.y.za"
```

After a little while the package is published now, so go to:

- https://launchpad.net/~fractorium/+archive/ubuntu/ppa
- Select "Package Details"
- Open the dropdown arrow at the package listing, see "Publishing Details"
- Under "Package Files", copy the link to the `.deb`

https://launchpad.net/~fractorium/+archive/ubuntu/ppa/+files/fractorium_w.x.y.za-0ubuntu1_amd64.deb

Update the README.md link to this.

All done. Commit, push, send Pull Request.
