# emscripten-compiler

## 動機

- emscripten の docker image は存在するけど (<https://hub.docker.com/r/emscripten/emsdk/tags>) サイズ大きすぎ
- 別の人が管理している docker image もあるけど (<https://hub.docker.com/r/trzeci/emscripten-slim/tags>) 最新版にどうも対応してなさそう
- 単純に emscripten をサーバ側でビルドしたい

なので docker を勉強して docker image を作ってみたとさ

## 参考にしたリポジトリ

- [trzecieu/emscripten-docker](https://github.com/trzecieu/emscripten-docker)
