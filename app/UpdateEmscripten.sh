#!/bin/bash

function fetch_versions() {
  echo `curl -s "$1" | jq --raw-output "$2" | awk '/^[0-9.]+$/'`
}

function contains() {
  local source_array=$2
  for j in ${source_array[@]}
  do
    if [ $j == $1 ]
    then
      return 1
    fi
  done
  return 0
}

function do_update() {
  local version=$1
  git add Dockerfile
  set +o pipefail
  git commit -m "Update: CircleCI has updated emscripten to ${version}" | true
  git tag -a ${version} -m "emscripten ${version}" | true
  set -o pipefail
  git push origin --tags
  git push origin
}

EmscriptenVersionsInGithub=(`fetch_versions https://api.github.com/repos/emscripten-core/emscripten/tags '.[].name'`)
EmscriptenVersionsInDockerhub=(`fetch_versions https://registry.hub.docker.com/v2/repositories/nokotan/emscripten-compiler/tags/ '.results[].name'`)

LatestVersion=${EmscriptenVersionsInGithub[0]}
echo ${EmscriptenVersionsInGithub[@]}

if contains "${EmscriptenVersionsInDockerhub[@]}" $LatestVersion
then
  echo "version ${LatestVersion} is not found in github, will push to github"
  sed -i -e "s/ARG EMSCRIPTEN_VERSION=.*$/ARG EMSCRIPTEN_VERSION=${LatestVersion}/g" Dockerfile
  do_update $LatestVersion
fi