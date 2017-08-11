# preprocessor-debug
Observe how the C preprocessor expands your complicated macros


## Dependencies:
* clang 4.0
* CMake 3.3+


## Build instructions:
* `mkdir build && cd build`
* `cmake ..`
* `make`

## Use instructions:
`./pp-step ../example.cpp`


# asm.js version:
## Dependencies
* clang 4.0
* [yarn](https://yarnpkg.com/en/docs/install)
* Emscripten

## Build instructions:
* Build clang in bitcode. This is hard, but can be done with the help of [`emmake`](https://kripken.github.io/emscripten-site/docs/compiling/Building-Projects.html#building-projects)
* `cd web`
* `make`
* `yarn`

Since building clang in bitcode is hard, a pre-compiled version of the tool is in `web/compiled.tgz`
