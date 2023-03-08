FROM nokotan/emscripten-compiler:3.1.20

COPY include/ /include/

RUN echo "## Generate .pch file for OpenSiv3D v0.4" \
    && . ${EMSDK}/emsdk_set_env.sh \
    && cd /include/OpenSiv3D \
    && emcc -O0 -fPIC -fvisibility=default -std=c++17 -I/include/OpenSiv3D -I/include/OpenSiv3D/ThirdParty Siv3D.hpp -o Siv3D.O0.pch \
    && emcc -O2 -fPIC -fvisibility=default -std=c++17 -I/include/OpenSiv3D -I/include/OpenSiv3D/ThirdParty Siv3D.hpp -o Siv3D.O2.pch \
    && emcc -Oz -fPIC -fvisibility=default -std=c++17 -I/include/OpenSiv3D -I/include/OpenSiv3D/ThirdParty Siv3D.hpp -o Siv3D.Oz.pch \
    \
    # cleanup
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
&& echo "## Done"

RUN echo "## Generate .pch file for OpenSiv3D v0.6" \
    && . ${EMSDK}/emsdk_set_env.sh \
    && cd /include/OpenSiv3Dv0.6 \
    && emcc -O0 -fPIC -fvisibility=default -std=c++2a -I/include/OpenSiv3Dv0.6 -I/include/OpenSiv3Dv0.6/ThirdParty -D_XM_NO_INTRINSICS_ Siv3D.hpp -o Siv3D.O0.pch \
    && emcc -O2 -fPIC -fvisibility=default -std=c++2a -I/include/OpenSiv3Dv0.6 -I/include/OpenSiv3Dv0.6/ThirdParty -D_XM_NO_INTRINSICS_ Siv3D.hpp -o Siv3D.O2.pch \
    && emcc -Oz -fPIC -fvisibility=default -std=c++2a -I/include/OpenSiv3Dv0.6 -I/include/OpenSiv3Dv0.6/ThirdParty -D_XM_NO_INTRINSICS_ Siv3D.hpp -o Siv3D.Oz.pch \
    \
    # cleanup
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
&& echo "## Done"

RUN echo "## Generate .pch file for DxLib" \
    && . ${EMSDK}/emsdk_set_env.sh \
    && cd /include/DxLib \
    && emcc -O0 -fPIC -fvisibility=default -std=c++17 -I/include/DxLib DxLib.h -o DxLib.O0.pch \
    && emcc -O2 -fPIC -fvisibility=default -std=c++17 -I/include/DxLib DxLib.h -o DxLib.O2.pch \
    && emcc -Oz -fPIC -fvisibility=default -std=c++17 -I/include/DxLib DxLib.h -o DxLib.Oz.pch \
    \
    # cleanup
    &&  find ${EMSDK} -name "*.pyc" -exec rm {} \; \
&& echo "## Done"

# Arbitrary folder, nothing special here
WORKDIR /src

# ------------------------------------------------------------------------------
# Copy this Dockerimage into image, so that it will be possible to recreate it later
COPY Dockerfile /app/

COPY app/ /app/

LABEL maintainer="kamenokonokotan@gmail.com" \
      org.label-schema.name="emscripten-compiler" \
      org.label-schema.description="This image includes compiler slug." \
      org.label-schema.url="https://github.com/nokotan/emscripten-compiler" \
      org.label-schema.vcs-url="https://github.com/nokotan/emscripten-compiler" \
      org.label-schema.docker.dockerfile="/app/Dockerfile"

SHELL ["/bin/bash", "-c"]
CMD node /index.js
