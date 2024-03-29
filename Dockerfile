# Copyright (c) 2021 kamenokonokotan
# Copyright (c) 2017 Piotr Paczkowski
# Licensed under the MIT license

FROM debian:buster AS stage_build

# ------------------------------ ABOUT THIS IMAGE  -----------------------------
# This Dockerfile has two major sections:
# * STAGE BUILD: Which uses required tools to build a static version of Emscripten SDK
# * STAGE DEPLOY: Which copies folder of Emscripten (/emsdk_portable) from previous stage, and installs very based tools to make Emscripten work.
#
# Compiled Emscripten SDK meant to be ready to go out of the shelf. That is `/emsdk_portable`:
# - contains every required part of Emscripten SDK
# - contains entrypoint that should be used for derived images
# - is able to work with further changes coming from SDK updates (`emsdk install ...`, etc)
# - contains some useful symbolic links that makes sure you can use Emscripten SDK in the same way, regardless of version that it holds
# Created symbolic links:
# - `/emsdk_portable/emscripten/sdk`: Points to folder that holds Emscripten SDK tools like `emcc`, (example: `/emsdk_portable/emscripten/tag-1.38.31`)
# - `/emsdk_portable/binaryen/bin`: (example: `/emsdk_portable/tag-1.38.31_64bit_binaryen/bin`)
# - `/emsdk_portable/llvm/clang`: Emscripten version of Clang (example: `/emsdk_portable/clang/tag-1.38.31/build_tag-1.38.31_64`)
# - `/emsdk_portable/node/current`: Embedded version of NodeJS (example: `/emsdk_portable/node/8.9.1_64bit`)

# ------------------------------------------------------------------------------
# -------------------------------- STAGE BUILD  --------------------------------
# ------------------------------------------------------------------------------
ARG EMSCRIPTEN_VERSION=3.1.32
ARG EMSCRIPTEN_SDK=${EMSCRIPTEN_VERSION}
ARG EMSDK_CHANGESET=main

# ------------------------------------------------------------------------------
# Following variables are important to tell Emscripten to use pre-defined locations
# for loading config file and place cache files. Otherwise SDK will use own folders under `/root/` folder
ENV EMSDK /emsdk_portable
ENV EM_DATA ${EMSDK}/.data
ENV EM_CONFIG ${EMSDK}/.emscripten
ENV EM_CACHE ${EM_DATA}/cache
ENV EM_PORTS ${EM_DATA}/ports
# ------------------------------------------------------------------------------

RUN echo "## Update and install packages" \
    &&  apt-get -qq -y update && apt-get -qq install -y --no-install-recommends \
        libxml2 \
        wget \
        git-core \
        ca-certificates \
        build-essential \
        file \
        python3 \
        python3-pip \
    &&  rm -rf /usr/bin/python \
    &&  ln -s /usr/bin/python3 /usr/bin/python \
&&  echo "## Done"


RUN  echo "## Get EMSDK" \
    &&  git clone https://github.com/emscripten-core/emsdk.git ${EMSDK} && cd ${EMSDK} && git reset --hard ${EMSDK_CHANGESET} \
        \
    &&  chmod +x ./emsdk.py \
    &&  ./emsdk.py update-tags \
&&  echo "## Done"


RUN echo "## Install Emscripten" \
    &&  cd ${EMSDK} \
        \
    # Compile llvm with dynamic libs support
    # This will create local shared objects that are used on all LLVM tool.
    # It significantly reduces final size of image.
    &&  ./emsdk install --build=MinSizeRel ${EMSCRIPTEN_SDK} \
        \
&&  echo "## Done"

# ------------------------------------------------------------------------------
# This generates configuration that contains all valid paths according to installed SDK
RUN cd ${EMSDK} \
    &&  echo "## Generate standard configuration" \
        \
    &&  ./emsdk activate ${EMSCRIPTEN_SDK} --embedded \
        \
    &&  touch emsdk_set_env.sh \
    # Export this image specific Environment variables
    # Those variables are important to use dedicated folder for all cache and predefined config file
    &&  printf "export EM_DATA=${EM_DATA}\n"                            >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export EM_CONFIG=${EM_CONFIG}\n"                        >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export EM_CACHE=${EM_CACHE}\n"                          >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export EM_PORTS=${EM_PORTS}\n"                          >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "\n"                                                     >> ${EMSDK}/emsdk_set_env.sh \
        \
    # Import default emsdk config
    &&  . ${EMSDK}/emsdk_env.sh \
    &&  printf "export EMSDK=${EMSDK}\n"                                >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export EMSDK_NODE=${EMSDK_NODE}\n"                      >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export EMSDK_PYTHON=${EMSDK_PYTHON}\n"                  >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export PATH=$(dirname $(which emcc)):\${PATH}\n"        >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export PATH=$(dirname $(which node)):\${PATH}\n"        >> ${EMSDK}/emsdk_set_env.sh \
    &&  printf "export PATH=${EMSDK}/upstream/bin:\${PATH}\n"             >> ${EMSDK}/emsdk_set_env.sh \
        \
&&  echo "## Done"


# ------------------------------------------------------------------------------
# Create a structure and make mutable folders accessible for r/w
RUN cd ${EMSDK} \
    &&  echo "## Create .data structure" \
    &&  for mutable_dir in ${EM_DATA} ${EM_PORTS} ${EM_CACHE} ${EMSDK}/zips ${EMSDK}/tmp; do \
            mkdir -p ${mutable_dir}; \
            chmod -R 777 ${mutable_dir}; \
        done \
        \
&&  echo "## Done"

# Step uses some bashizm
SHELL ["/bin/bash", "-c"]
RUN cd ${EMSDK} \
    &&  echo "## Clean-up Emscripten Installation" \
    # Allow to use ** for recursive wildcard - bash only!
    &&  shopt -s globstar \
        \
    &&  find . -name "*.pyc" -exec rm {} \; \
    &&  find . -name "CMakeFiles" -type d -prune -exec rm -fr {} \; \
    &&  find . -name "CMakeCache.txt" -exec rm {} \; \
        \
    &&  find . -name "*.o" -exec rm {} \; \
    &&  find . -name "*.a" -exec rm {} \; \
    # &&  find . -name "*.inc*" -exec rm {} \; \
    &&  find . -name "*.gen.tmp" -exec rm {} \; \
        \
    &&  rm -rf **/*_32bit \
    &&  rm -rf **/.git \
    &&  rm -rf **/tests \
    &&  rm -rf **/docs \
    &&  rm -fr zips/* \
        \
    &&  rm -fr /opt/cmake /usr/local/bin/cmake /cmake* \
        \
    # sleep will make sure that created cache will be stored correctly
    &&  sleep 2 \
&&  echo "## Done"

RUN apt-get -qq -y update && apt-get -qq install -y --no-install-recommends \
        binutils \
    && . ${EMSDK}/emsdk_set_env.sh \
    # Remove debugging symbols from embedded node (extra 7MB)
    && strip -s `which node` \
    # strip out symbols from clang (extra 50MB!)
    && find ${EMSDK}/upstream/bin -type f -exec strip -s {} + &> /dev/null || true \
    && find ${EMSDK}/upstream/lib -type f -exec strip -s {} + &> /dev/null || true \
&&  echo "## Done"

# ------------------------------------------------------------------------------

RUN echo "## Create transferable entrypoint" \
    &&  printf '#!/bin/bash\n'                                            >  ${EMSDK}/entrypoint \
    # In case when mapped user id by `docker run -u` is not created inside docker image
    # The `$HOME` variable points to `/` - which prevents any tool to write to, as it requires root access
    # In such case we set `$HOME` to `/tmp` as it should r/w for everyone
    &&  printf 'if [ "$HOME" = "/" ] ; then\n'                          >> ${EMSDK}/entrypoint \
    &&  printf '    export HOME=/tmp\n'                                 >> ${EMSDK}/entrypoint \
    &&  printf 'fi\n'                                                   >> ${EMSDK}/entrypoint \
    &&  printf '\n'                                                     >> ${EMSDK}/entrypoint \
    # In case of running as root, use `umask` to reduce problem of file permission on host
    &&  printf 'if [ "$(id -g)" = "0" ] && [ "$(id -u)" = "0" ] ;\n'    >> ${EMSDK}/entrypoint \
    &&  printf 'then\n'                                                 >> ${EMSDK}/entrypoint \
    &&  printf '    umask 0000\n'                                       >> ${EMSDK}/entrypoint \
    &&  printf 'fi\n'                                                   >> ${EMSDK}/entrypoint \
    \
    # Activate Emscripten SDK
    &&  printf '. ${EMSDK}/emsdk_set_env.sh > /dev/null\n'              >> ${EMSDK}/entrypoint \
    # Evaluate a command that's coming after `docker run` / `docker exec`
    &&  printf '"$@"\n'                                                 >> ${EMSDK}/entrypoint \
    \
    &&  chmod +x ${EMSDK}/entrypoint \
    \
&&  echo "## Done"


# Populate Emscripten SDK cache with libc++, to improve further compilation times.
RUN echo "## Pre-populate cache" \
    &&  . ${EMSDK}/emsdk_set_env.sh \
    \
    &&  mkdir -p /tmp/emscripten_test \
    &&  cd /tmp/emscripten_test \
    \
        &&  printf '#include <iostream>\nint main(){std::cout << "HELLO FROM DOCKER C++"<<std::endl;return 0;}' > test.cpp \
        # &&  em++ --std=c++11 test.cpp -o test.js -s WASM=0 &&  node test.js \
        # &&  em++ --std=c++11 -g4 test.cpp -o test.js -s WASM=0 &&  node test.js \
        &&  em++ --std=c++11 test.cpp -o test.js -s WASM=1 -s USE_SDL=2 &&  node test.js \
        &&  em++ --std=c++11 test.cpp -o test.js -s WASM=1 -s MAIN_MODULE=1 &&  node test.js \
    \
    &&  cd / \
    &&  rm -fr /tmp/emscripten_test \
    \
    # some files were created, and we need to make sure that those can be accessed by non-root people
    &&  chmod -R 777 ${EM_DATA} \
    \
    # cleanup
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
    \
    &&  echo "## Done"

# Create symbolic links for critical Emscripten Tools
# This is important for letting people using Emscripten in Dockerfiles without activation
# As each Emscripten release is placed to a different folder (i.e. /emsdk_portable/emscripten/tag-1.38.31)
# We need to somehow make it fixed. An old solution was to move folders around but it has some drawbacks.
# Current solution is to create symlinks that matches old solution: which maintains compatibility
# The ultimate goal is to simplify a way to use Emscripten SDK without a need to activate it.
RUN echo "## Create symbolic links" \
    &&  . ${EMSDK}/emsdk_set_env.sh \
    \
    &&  mkdir -p ${EMSDK}/llvm ${EMSDK}/emscripten ${EMSDK}/binaryen \
    \
    &&  echo $(which node) && ln -s $(dirname $(which node))/..         ${EMSDK}/node/current \
    &&  echo $(which clang) && ln -s $(dirname $(which clang))/..       ${EMSDK}/llvm/clang \
    &&  echo $(which emcc) && ln -s $(dirname $(which emcc))            ${EMSDK}/emscripten/sdk \
    &&  echo $(which wasm-opt) && ln -s $(dirname $(which wasm-opt))    ${EMSDK}/binaryen/bin \
    \
    &&  echo "## Done"

# ------------------------------------------------------------------------------
# -------------------------------- STAGE DEPLOY --------------------------------
# ------------------------------------------------------------------------------
FROM debian:buster-slim AS stage_deploy

COPY --from=stage_build /emsdk_portable /emsdk_portable

# Fallback in case Emscripten isn't activated.
# This will let use tools offered by this image inside other Docker images (sub-stages) or with custom / no entrypoint
ENV EMSDK /emsdk_portable
ENV EMSCRIPTEN=${EMSDK}/emscripten/sdk

ENV EM_DATA ${EMSDK}/.data
ENV EM_CONFIG ${EMSDK}/.emscripten
ENV EM_CACHE ${EM_DATA}/cache
ENV EM_PORTS ${EM_DATA}/ports

# Fallback in case Emscripten isn't activated
# Expose Major tools to system PATH, so that emcc, node, asm2wasm etc can be used without activation
ENV PATH="${EMSDK}:${EMSDK}/upstream/bin:${EMSDK}/emscripten/sdk:${EMSDK}/llvm/clang/bin:${EMSDK}/node/current/bin:${EMSDK}/binaryen/bin:${PATH}"

ENV LANG=C.UTF-8

# Use entrypoint that's coming from emscripten-slim image. It sets all required system paths and variables
ENTRYPOINT ["/emsdk_portable/entrypoint"]

# ------------------------------------------------------------------------------

# Create a 'standard` 1000:1000 user
# Thanks to that this image can be executed as non-root user and created files will not require root access level on host machine
# Please note that this solution even if widely spread (even node Dockerimages use that) is far from perfect as user 1000:1000 might not exist on
# host machine, and in this case running any docker image will cause other random problems (mostly due `$HOME` pointing to `/`)
# This extra user works nicely with entrypoint provided in `/emsdk_portable/entrypoint` as it detects case explained before.
RUN echo "## Create emscripten user (1000:1000)" \
    &&  groupadd --gid 1000 emscripten \
    &&  useradd --uid 1000 --gid emscripten --shell /bin/bash --create-home emscripten \
    \
&&  echo "## Done"


RUN echo "## Update and install packages" \
&&  apt-get -qq -y update && apt-get -qq install -y --no-install-recommends \
        libxml2 \
        ca-certificates \
        python3 \
        python3-pip \
    \
    &&  rm -rf /usr/bin/python \
    &&  ln -s /usr/bin/python3 /usr/bin/python \
    # Standard Cleanup on Debian images
    &&  apt-get -y clean \
    &&  apt-get -y autoclean \
    &&  apt-get -y autoremove \
    &&  rm -rf /var/lib/apt/lists/* \
    &&  rm -rf /var/cache/debconf/*-old \
    &&  rm -rf /usr/share/doc/* \
    &&  rm -rf /usr/share/man/?? \
    &&  rm -rf /usr/share/man/??_* \
&&  echo "## Done"

# Docker's convention is to create entrypoint in /entrypoint path.
# Let's create this entrypoint for compatibility and to keep tradition, which forward command to the real entrypoint
# Main intentions is to keep compatibility with previous images
RUN echo "## Create standard docker entrypoint" \
    &&  printf '#!/bin/bash\n'                  >  /entrypoint \
    &&  printf ". ${EMSDK}/entrypoint \"$@\"\n"     >> /entrypoint \
    \
    &&  chmod +x /entrypoint \
    \
&&  echo "## Done"

# Arbitrary folder, nothing special here
WORKDIR /src

# ------------------------------------------------------------------------------
# Copy this Dockerimage into image, so that it will be possible to recreate it later
COPY Dockerfile /emsdk_portable/dockerfiles/trzeci/emscripten-slim/

LABEL maintainer="kamenokonokotan@gmail.com" \
      org.label-schema.name="emscripten-slim" \
      org.label-schema.description="This image includes EMSDK, Emscripten and WebAssembly compiler and tools that are very required to compile sources."

# ------------------------- POST BUILD IN-PLACE TESTING ------------------------

SHELL ["/bin/bash", "-c"]
RUN echo "## Internal Testing of image (activated)" \
    &&  . ${EMSDK}/emsdk_set_env.sh \
    &&  set -x \
    # binaryen
    &&  which wasm-opt \
    # clang
    &&  which llvm-ar \
    # emscritpen
    &&  which emsdk \
    \
    &&  node --version \
    &&  npm --version \
    &&  python3 --version \
    &&  pip3 --version \
    \
    &&  em++ --version \
    &&  emcc --version \
    \
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
    \
&&  echo "## Done"

RUN echo "## Internal Testing of image (no activation)" \
    &&  set -x \
    # binaryen
    &&  which wasm-opt \
    # clang
    &&  which llvm-ar \
    # emscritpen
    &&  which emsdk \
    \
    &&  node --version \
    &&  npm --version \
    &&  python3 --version \
    &&  pip3 --version \
    \
    &&  em++ --version \
    &&  emcc --version \
    \
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
    \
&&  echo "## Done"

# ------------------------------------------------------------------------------
