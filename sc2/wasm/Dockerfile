# Run from sc2/ directory:
# docker build --tag=uqm-wasm -f wasm/Dockerfile .
# docker run -it --rm -p9999:9999 uqm-wasm

FROM emscripten/emsdk:latest
RUN apt-get update && \
    apt-get install -y python-is-python3 nginx && \
    rm -rf /var/lib/apt/lists/*
# Pre-build ports
RUN emcc -c -E - -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_SDL=2 -s ASYNCIFY -pthread </dev/null

COPY build ./build
COPY src ./src
COPY Make* build.sh build.vars.in ./

RUN emconfigure ./build.sh uqm config

COPY content ./content
RUN ./build.sh uqm -j$(nproc) && \
    rm -rf obj
COPY wasm ./wasm

CMD nginx -p $PWD -c wasm/nginx.conf
