# emscripten-compiler-server

emcc compiler server

## 参考にしたリポジトリ

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
