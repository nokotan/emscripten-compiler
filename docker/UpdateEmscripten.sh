#!/bin/bash

Versions=(`curl -s https://api.github.com/repos/emscripten-core/emscripten/tags | jq --raw-output .[].name | awk '/^[0-9.]+$/'`)
LatestVersion=${Versions[0]}

# Update
sed -i -e "s/ARG EMSCRIPTEN_VERSION=.*$/ARG EMSCRIPTEN_VERSION=${LatestVersion}/g" Dockerfile
git add Dockerfile
set +o pipefail
git commit -m "Update: CircleCI has updated emscripten to ${LatestVersion}" | true
git tag -a ${LatestVersion} -m "emscripten ${LatestVersion}" | true
set -o pipefail
git push origin --tags
git push origin