FROM nokotan/emscripten-compiler:3.1.20

# Arbitrary folder, nothing special here
WORKDIR /src

# ------------------------------------------------------------------------------
# Copy this Dockerimage into image, so that it will be possible to recreate it later
COPY Dockerfile /

COPY index.js config.js /app/

COPY src/ /app/src/

LABEL maintainer="kamenokonokotan@gmail.com" \
      org.label-schema.name="emscripten-compiler-app" \
      org.label-schema.description="This image includes compiler slug." \
      org.label-schema.url="https://github.com/nokotan/emscripten-compiler" \
      org.label-schema.vcs-url="https://github.com/nokotan/emscripten-compiler" \
      org.label-schema.docker.dockerfile="/Dockerfile"

SHELL ["/bin/bash", "-c"]
CMD node /app/index.js
