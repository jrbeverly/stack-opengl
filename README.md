# stack-opengl

## Summary

stack-opengl is a variant of `stack-net` written in OpenGL. It uses extremely simple shaders and OpenGL programming to create a block stacking application.

## Getting Started

The project uses **premake4** as the cross-platform build system. You will need to build the external dependencies of the project, by running a root level build. You can then build the project itself. You can do so as such:

```bash
premake4 gmake
make

cd src/
premake4 gmake
make
./Stack
```

## Acknowledgements

The project icon is retrieved from [the Noun Project](docs/icon/icon.json). The original source material has been altered for the purposes of the project. The icon is used under the terms of the [Public Domain](https://creativecommons.org/publicdomain/zero/1.0/).

The project icon is by [Christina Witt George from the Noun Project](https://thenounproject.com/term/cube/4025/).