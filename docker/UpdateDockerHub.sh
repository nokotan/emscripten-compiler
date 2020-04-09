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
  local image_name=$1
  docker build -t $image_name .
  docker push $image_name
}

EmscriptenVersionsInGithub=(`fetch_versions https://api.github.com/repos/emscripten-core/emscripten/tags 'reverse | .[].name'`)
EmscriptenVersionsInDockerhub=(`fetch_versions https://registry.hub.docker.com/v2/repositories/nokotan/emscripten-compiler/tags/ '.results | reverse | .[].name'`)

for i in ${EmscriptenVersionsInGithub[@]}
do
  if contains "${EmscriptenVersionsInDockerhub[@]}" $i && 
  then
    echo "version ${i} is not found in dockerhub, will push to dockerhub"
    sed -i -e "s/ARG EMSCRIPTEN_VERSION=.*$/ARG EMSCRIPTEN_VERSION=${i}/g" Dockerfile
    do_update nokotan/emscripten-compiler:$i
    exit
  fi
done