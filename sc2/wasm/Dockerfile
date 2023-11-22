# Run from sc2/ directory:
# docker build --tag=uqm-wasm -f wasm/Dockerfile .
# docker run -it --rm -p9999:9999 uqm-wasm

FROM emscripten/emsdk
RUN apt-get update && \
    apt-get install -y python-is-python3 nginx && \
    rm -rf /var/lib/apt/lists/*
# Pre-build ports
RUN emcc -c -E - -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_SDL=2 -s ASYNCIFY -pthread </dev/null

COPY build ./build
COPY src ./src
COPY Make* build.sh build.vars.in ./
COPY wasm ./wasm

RUN emconfigure ./build.sh uqm config

COPY content ./content
RUN ./build.sh uqm -j$(nproc) && \
    rm -rf obj
RUN ls -lah uqm-debug.wasm uqm-debug.data

CMD nginx -p $PWD -c wasm/nginx.conf
