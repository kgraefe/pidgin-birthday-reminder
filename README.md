Pidgin Birthday Reminder
========================

Installation on Windows
-----------------------

For the binary version, extract the all contents of the ZIP file either to the
installation directory of Pidgin (typically `C:\Program Files\Pidgin`) or to
your .purple user directory (typically `%APPDATA%\Roaming\.purple`).

See below for how to compile the source code version into a binary ZIP file.

Installation on Linux
---------------------

To install the plugin extract the binary tar archive and copy all directories
into your .purple user directory (usually `~/.purple`), e.g.:

    tar xzvf pidgin-birthday-reminder-1.7-linux-x86_64.tar.gz
    cd pidgin-birthday-reminder-1.7-linux-x86_64
    cp -r * ~/.purple/

**Note:** Your hardware platform has to match the archive's. Check with `uname --hardware-platform`.

**Note:** For a system-wide installation the plugin needs to be built from
source (see below).

Building on Windows
-------------------

In order to build the plugin for Windows an already-compiled source tree of
Pidgin is required. Please see the [Pidgin for Windows Build Instructions](https://developer.pidgin.im/wiki/BuildingWinPidgin)
for details.

After that you need to create a file named `local.mak` that points to the Pidgin source tree, e.g.:

    PIDGIN_TREE_TOP=$(PLUGIN_TOP)/../../pidgin-2.10.11

Now you can build the plugin:

    make -f Makefile.mingw dist

Building on Linux
-----------------

    sudo apt-get install pidgin-dev
    ./autogen.sh
    ./configure
    make
    sudo make install

**Note:** In order to use a self-compiled plugin with a Pidgin installed through
your package manager, you need to install it into `/usr` in most cases (default
is `/usr/local`). Use `./configure --prefix=/usr` in this case.
