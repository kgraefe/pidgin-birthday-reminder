Pidgin Burthday Reminder
========================

Building on Linux
-----------------

    sudo apt-get install pidgin-dev
    autoreconf -i
    ./configure
    make
    sudo make install


Building on Windows
-------------------

In order to build the plugin for Windows an already-compiled source tree of
Pidgin is required. Please see the [Pidgin for Windows Build Instructions](https://developer.pidgin.im/wiki/BuildingWinPidgin)
for details.

After that you need to create a file named `local.mak` that points to the Pidgin source tree, e.g.:

    PIDGIN_TREE_TOP=$(PLUGIN_TOP)/../../pidgin-2.10.11

Now you can build the plugin:

    make -f Makefile.mingw
