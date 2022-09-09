const endpoint = "https://emscripten-compiler.herokuapp.com/build";

import * as fetch from "node-fetch";

async function sendJson(content) {
    const body = JSON.stringify(content);

    const response = await fetch.default(endpoint, {
        method: "POST",
        body
    });

    const text = await response.text();

    if (response.ok) {
        return JSON.parse(text);
    } else {
        console.error(`Status Code: ${response.status}`);
        console.log(text);
        throw new Error();
    }
}

function waitForMilliSeconds(timeout) {
    return new Promise(resolve => {
        setTimeout(resolve, timeout);
    });
}

function factorial(depth) {
    if (depth <= 1) return 1;
    return factorial(depth - 1) * depth;
}

class PoissonDistributionRandom {
    /**
     * 
     * @param {number} expectedTrialInTimeSlice 
     * @param {number} maxTrial 
     */
    constructor(expectedTrialInTimeSlice, maxTrial) {
        /** @type {number[]} */
        this._distibutionList = [ ];
        let accumulated = 0.0;

        for (let i = 0; i < maxTrial; i++) {
            accumulated += Math.pow(expectedTrialInTimeSlice, i) * Math.exp(-expectedTrialInTimeSlice) / factorial(i);
            this._distibutionList.push(accumulated);
        }

        this._distibutionList.push(1.0);
    }

    /** @returns {number} */
    get() {
        const seed = Math.random();
        for (let i = 0; i < this._distibutionList.length; i++) {
            if (seed <= this._distibutionList[i]) {
                return i;
            }
        }
    }
}

let times = [];

async function episode() {
    const t0 = performance.now();

    const result = await sendJson({
        output: "wasm",
        compress: true,
        files: [
            {
                type: "cpp",
                name: "Main.cpp",
                options: "-Wall -std=c++2a -O2 -include-pch /include/OpenSiv3Dv0.6/Siv3D.O2.pch -I/include/OpenSiv3Dv0.6/ -I/include/OpenSiv3Dv0.6/ThirdParty/ -s SIDE_MODULE=1 -D_XM_NO_INTRINSICS_ -s ASYNCIFY=1 -s ASYNCIFY_IMPORTS=\"[ '_ZN3s3d6System6UpdateEv' ]\"",
                src: `
                    # include <Siv3D.hpp> // OpenSiv3D v0.6

                    # if defined(SIV3D_WEBGPU_BACKEND)
                    SIV3D_SET(EngineOption::Renderer::WebGPU)
                    # endif
                    
                    void Main()
                    {
                        // 背景の色を設定 | Set background color
                        Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });
                    
                        // 通常のフォントを作成 | Create a new font
                        const Font font{ 60 };
                    
                        // 絵文字用フォントを作成 | Create a new emoji font
                        const Font emojiFont{ 60, Typeface::ColorEmoji };
                    
                        // \`font\` が絵文字用フォントも使えるようにする | Set emojiFont as a fallback
                        font.addFallback(emojiFont);
                    
                        // 画像ファイルからテクスチャを作成 | Create a texture from an image file
                        const Texture texture{ U"example/windmill.png" };
                    
                        // 絵文字からテクスチャを作成 | Create a texture from an emoji
                        const Texture emoji{ U"🐈"_emoji };
                    
                        // 絵文字を描画する座標 | Coordinates of the emoji
                        Vec2 emojiPos{ 300, 150 };
                    
                        // テキストを画面にデバッグ出力 | Print a text
                        Print << U"Push [A] key";
                    
                        while (System::Update())
                        {
                            // テクスチャを描く | Draw a texture
                            texture.draw(200, 200);
                    
                            // テキストを画面の中心に描く | Put a text in the middle of the screen
                            font(U"Hello, Siv3D!🚀").drawAt(Scene::Center(), Palette::Black);
                    
                            // サイズをアニメーションさせて絵文字を描く | Draw a texture with animated size
                            emoji.resized(100 + Periodic::Sine0_1(1s) * 20).drawAt(emojiPos);
                    
                            // マウスカーソルに追随する半透明な円を描く | Draw a red transparent circle that follows the mouse cursor
                            Circle{ Cursor::Pos(), 40 }.draw(ColorF{ 1, 0, 0, 0.5 });
                    
                            // もし [A] キーが押されたら | When [A] key is down
                            if (KeyA.down())
                            {
                                // 選択肢からランダムに選ばれたメッセージをデバッグ表示 | Print a randomly selected text
                                Print << Sample({ U"Hello!", U"こんにちは", U"你好", U"안녕하세요?" });
                            }
                    
                            // もし [Button] が押されたら | When [Button] is pushed
                            if (SimpleGUI::Button(U"Button", Vec2{ 640, 40 }))
                            {
                                // 画面内のランダムな場所に座標を移動
                                // Move the coordinates to a random position in the screen
                                emojiPos = RandomVec2(Scene::Rect());
                            }
                        }
                    }`
            }
        ],
        link_options: "-Wall -std=c++2a -O2 -include-pch /include/OpenSiv3Dv0.6/Siv3D.O2.pch -I/include/OpenSiv3Dv0.6/ -I/include/OpenSiv3Dv0.6/ThirdParty/ -s SIDE_MODULE=1 -D_XM_NO_INTRINSICS_ -s ASYNCIFY=1 -s ASYNCIFY_IMPORTS=\"[ '_ZN3s3d6System6UpdateEv' ]\""
    });

    const t1 = performance.now();
    const elapsedTime = t1 - t0;
    times.push(elapsedTime);
    console.log(`took ${elapsedTime}`);

    console.assert(result.success);
}

async function main() {
    const interval = 100;
    const testDuration = 10000;
    const loopTrial = testDuration / interval;

    let totalTrial = 0;
    const generator = new PoissonDistributionRandom(3 / (testDuration / interval), 3);
    const taskPromises = [];

    for (let i = 0; i < loopTrial; i++) {
        const trialNum = generator.get();

        if (trialNum > 0) {
            console.log(`${100 * i}: ${trialNum} build task occurred.`);
        }

        totalTrial += trialNum;

        for (let j = 0; j < trialNum; j++) {
            taskPromises.push(episode());
        }

        await waitForMilliSeconds(interval);
    }

    await Promise.all(taskPromises);

    console.log(`Total trial: ${totalTrial}`);
    console.log(`Max Time: ${Math.max(...times)}`);
    console.log(`Min Time: ${Math.min(...times)}`);
}

main();
