# Stack.OpenGL

## Summary

## Building Projects

We use **premake4** as our cross-platform build system. First you will need to build all
the static libraries that the projects depend on. To build the libraries, open up a
terminal, and **cd** to the top level of the CS488 project directory and then run the
following:

```bash
premake4 gmake
make
```

This will build the following static libraries, and place them in the top level **lib**
folder of your cs488 project directory.

    * libcs488-framework.a
    * libglfw3.a
    * libimgui.a

Next we can build a specific project.  To do this, **cd** into one of the project folders,
say **Stack** for example, and run the following terminal commands in order to compile the `Stack` executable using all .cpp files in the `Stack` directory:

```bash
cd Stack/
premake4 gmake
make
```

## Windows

Sorry for all of the hardcore Microsoft fans out there.  We have not had time to test the build system on Windows yet. Currently our build steps work for OSX and Linux, but all the steps should be the same on Windows, except you will need different libraries to link against when building your project executables.  Some good news is that premake4 can output a Visual Studio .sln file by running:

```bash
premake4 vs2013
```

 This should point you in the general direction.

## Acknowledgements

The project icon is retrieved from [the Noun Project](docs/icon/icon.json). The original source material has been altered for the purposes of the project. The icon is used under the terms of the [Public Domain](https://creativecommons.org/publicdomain/zero/1.0/).

The project icon is by [Christina Witt George from the Noun Project](https://thenounproject.com/term/cube/4025/).