Star Control 2: The Ur-Quan Masters port to WebAssembly
-------------------------------------------------------

[![Build status](https://github.com/intgr/uqm-wasm/workflows/Build/badge.svg?branch=main)](https://github.com/intgr/uqm-wasm/actions?query=workflow:Build)

This is a fork of Star Control 2: The Ur-Quan Masters to WebAssembly
for running in web browsers, using the Emscripten toolchain.

Beware! In the WebAssembly build, **saved games are not** persistent, and the
game has not received much testing. It is not fit for a full play-through, but
currently merely a toy project.

Building in Docker
------------------
The easiest way to build and run UQM with Emscripten is via Docker.
In the `sc2` directory, Simply run:

    docker build --tag=uqm-wasm -f wasm/Dockerfile .
    docker run -it --rm -p9999:9999 uqm-wasm

Then open in your web browser: http://localhost:9999/uqm-debug.html

Building locally
----------------
* Install Emscripten: https://emscripten.org/docs/getting_started/downloads.html
* Make sure Emscripten binaries/wrappers are in PATH.
  The command `emcc --show-ports` should work.

Run the configuration menu:

    cd sc2
    emconfigure ./build.sh uqm config

The first time you run this, it takes a few minutes as Emscripten downloads
and builds ports of required libraries.

Nothing special needs to be done there, everything should be auto-detected.
Just for reference, here is a tested working configuration:

     1. Type of build                        Debugging build
     2. Graphics Engine                      SDL2 with modern graphics support
     3. Sound backend                        Include OpenAL support (experimental)
     4. Tracker music support                Included libmikmod
     5. Ogg Vorbis codec                     Xiph libogg + libvorbis
     6. Network Supermelee support           disabled
     7. Joystick support                     enabled
     8. Supported file i/o methods           Direct & .zip file i/o
     9. Graphics/Sound optimizations         Platform acceleration (asm, etc.)
    10. Thread library                       Pthread thread library

Press <ENTER> to finish configuration and build it:

    ./build.sh uqm -j16

You should have an `uqm.html` file as output.

Run it
------
You can use the included nginx configuration to serve up files for the web browser:

    nginx -p $PWD -c wasm/nginx.conf

Then open in your web browser: http://localhost:9999/uqm-debug.html

Known issues
------------
* Gameplay: No support for persistent saved games.
* Audio: Firefox doesn't play some samples (e.g. menu ping), but works in Chrome
* Audio: MixSDL does not work (OpenAL works)
* Threading: SDL threading does not work (pthreads work)

Troubleshooting
---------------
You can clean and force a re-build of emscripten-ports with:

    emcc --clear-cache
    emcc -c -E - -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_SDL=2 -s ASYNCIFY -pthread </dev/null

Credits
-------

* Thanks to Fred Ford, Paul Reiche III and Toys for Bob for releasing the game as open source.
* Thanks to The Ur-Quan Masters port team for their work on the open source project.
* Thanks to Michael Martin for continuing to maintain the game for the last several years.
* WebAssembly port was done by Marti Raudsepp.
* [Detailed credits](./sc2/AUTHORS)
