# Pidgin Birthday Reminder

[![Screenshot 1](https://cloud.githubusercontent.com/assets/3797324/11967672/16ac8a7c-a905-11e5-9e58-7b3bc856330a.png)](https://cloud.githubusercontent.com/assets/3797324/11967670/16aaf504-a905-11e5-9fca-3764fed95c39.png)
[![Screenshot 2](https://cloud.githubusercontent.com/assets/3797324/11967671/16abaf94-a905-11e5-8330-cad1e7118979.png)](https://cloud.githubusercontent.com/assets/3797324/11967669/16aa9014-a905-11e5-8a78-7fb248e1c0ea.png)
[![Screenshot 3](https://cloud.githubusercontent.com/assets/3797324/11967673/16b0aa30-a905-11e5-934a-5e0b02d5e51b.png)](https://cloud.githubusercontent.com/assets/3797324/11967674/16b12988-a905-11e5-98e7-37540049b24b.png)


## Installation on Windows

For the binary version, extract the all contents of the ZIP file either to the
installation directory of Pidgin (typically `C:\Program Files\Pidgin`) or to
your .purple user directory (typically `%APPDATA%\Roaming\.purple`).

See below for how to compile the source code version into a binary ZIP file.

## Installation on Linux

To install the plugin on Linux you need to compile it from source:

    sudo apt-get install pidgin-dev
    ./autogen.sh
    ./configure
    make
    sudo make install

**Note:** If you installed Pidgin through your package manager, you need to
install it into `/usr` in most cases (default is `/usr/local`). Use
`./configure --prefix=/usr` in this case.

## Building on Windows

In order to build the plugin for Windows an already-compiled source tree of
Pidgin is required. Please see the [Pidgin for Windows Build Instructions](https://developer.pidgin.im/wiki/BuildingWinPidgin)
for details.

After that you need to create a file named `local.mak` that points to the Pidgin source tree, e.g.:

    PIDGIN_TREE_TOP=$(PLUGIN_TOP)/../../pidgin-2.10.11

Now you can build the plugin:

    make -f Makefile.mingw dist

## Contributing

### Translate the plugin
Translations are managed at Transifex, please contribute there at
https://www.transifex.com/projects/p/pidgin-birthday-reminder/

### Contribute code
Please fork the project, do your changes and make pull request.

