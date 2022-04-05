# libsm64-supersonic-mario - Super Mario 64 as a library, for Suprsonic Mario Rocket League Mod

This repo is a fork of [libsm64](https://github.com/libsm64/libsm64) used for the Supersonic Mario Rocket League mod. Changes include supporting rendering a mario received from over the network, exposing audio flags, BLJ anywhere, and other miscellaneous changes.

## Building on Linux

- Ensure python3 is installed.
- Ensure the SDL2 and GLEW libraries are installed if you're building the test program (on Ubuntu: libsdl2-dev, libglew-dev)
- Run `make` to build
- To run the test program you'll need a SM64 US ROM in the root of the repository with the name `baserom.us.z64`.

## Building on Windows (test program not supported)
- [Follow steps 1-4 for setting up MSYS2 MinGW 64 here](https://github.com/sm64-port/sm64-port#windows), but replace the repository URL with `https://github.com/libsm64/libsm64.git`
- Run `make` to build

## Make targets (all platforms)

- `make lib`: (Default) Build the `dist` directory, containing the shared object or DLL and public-facing header.
- `make test`: Builds the library `dist` directory as well as the test program.
- `make run`: Build and run the SDL+OpenGL test program.

