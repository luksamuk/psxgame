# PSXGame

This is a template for a Playstation 1 game built using [PSn00bSDK](https://github.com/Lameguy64/PSn00bSDK).
It has been optimized to run and build on Linux (a build script using Docker
is planned).

It is loosely based on the project structure of [Sonic XA](https://luksamuk.itch.io/sonic-the-hedgehog-xa) ([see source code here](https://github.com/luksamuk/engine-psx)), a fangame for the PlayStation 1.

This template assumes that you have PSn00bSDK installed on your Linux system
under `/opt/psn00bsdk`, and the Makefile does all the heavy lifting of
leveraging environment variables and such.

Furthermore, it configures the CMake project to use Unix Makefiles instead
of Ninja, and is also ready for running the project with PCSX-Redux (which
must be on your PATH).

There is also a `.dir-locals.el` file which makes this project ready to be
used with Emacs and Flycheck, and also registers a DAP debugger for
`gdb-multiarch`, under the name "PSX Debug".

The root-level Makefile allows you to run the targets:

- `all` (or simply running `make`): Build the project and a CD image (.bin + .cue)
- `elf`: Build binary only
- `iso`: Build `elf` and CD image (.bin + .cue)
- `chd`: Build `iso`, then generate a single-file MAME disc image (.chd). Requires a tool called [`tochd`](https://github.com/thingsiplay/tochd).
- `run`: Build `iso`, then run it on PCSX-Redux.
- `configure`: Base configuration step for generating a `build` directory.
- `clean`: Remove `build` directory.
- `purge`: Same as `clean`, but also remove MAM disc image (.chd).
- `rebuild`: Force a complete rebuild of the project.


This template is distributed under the Mozilla Public License 2.0.

