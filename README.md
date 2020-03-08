# emscripten-compiler

## 動機

- emscripten の docker image は存在するけど (<https://hub.docker.com/r/trzeci/emscripten/>) バージョンが古い
- 同じ人が管理している dockerFile もあるけど (<https://github.com/trzecieu/emscripten-docker>) 最新版にどうも対応してなさそう
- 単純に emscripten をサーバ側でビルドしたい

なので docker を勉強して dockerFile を作ってみたとさ

## 参考にしたリポジトリ

- [trzecieu/emscripten-docker](https://github.com/trzecieu/emscripten-docker)
- [yurydelendik/clang-heroku-slug](https://github.com/yurydelendik/clang-heroku-slug)

## サーバへの post 例

### リクエスト側

```json
{
    "output": "wasm",
    "files": [
        {
            "type": "cpp",
            "name": "file.cpp",
            "options": "-O3 -std=c++14",
            "src": "int main() { return 0; }"
        }
    ],
    "link_options": ""
}
```

curl でタスクを投げる

```bash
curl -X POST -H 'Content-Type:application/json' -d '{
    "output": "wasm",
    "files": [
        {
            "type": "cpp",
            "name": "file.cpp",
            "options": "-O3 -std=c++14",
            "src": "int main() { return 0; }"
        }
    ],
    "link_options": ""
}' \
https://emscripten-compiler.herokuapp.com/build
```

### レスポンス側

```js
{
    "tasks": [
        {
            "name": "building file.cpp",
            "console": "...",
            "file": "file.cpp",
            "output": "AGFzbQEAAAAAD...",
            "success": true,
        },
        {
            "name": "linking wasm",
            "console": "...",
            "success": true,
        }
    ],
    "output": "AGFzbQEAAAAAD...",
    "jsFile": "Ly8gQ29weXJpZ...",
    "success": true,
    "message": "Success"
}
```

## そのほか

- <https://emscripten-compiler.herokuapp.com/build> でテストサーバ稼働中
- <https://nokotan.github.io/WebAssemblyStudio/> で試せるよ！
