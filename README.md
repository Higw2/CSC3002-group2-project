# CSC3002-group2-project
2D Platformer Game (C++ &amp; SFML)

项目结构：
EchoGidge/
├─ src/                # 源代码目录
│  └─ main.cpp         # 游戏主逻辑（控制、碰撞、渲染、音频调用）
├─ include/            # 头文件目录（扩展功能时可添加自定义头文件）
├─ assets/             # 游戏资源目录（自动同步到编译目录）
│  └─ audio/           # 音频资源子目录
├─ third-party/        # 内置第三方依赖库（无需额外下载）
│  └─ windows/
│     ├─ SDL2/         # SDL2 核心库（x86_64-w64-mingw32 架构）
│     ├─ SDL2_mixer/   # SDL2 音频扩展库
│     └─ SDL2_image/   # SDL2 图片扩展库（预留图片渲染功能）
├─ CMakeLists.txt      # 项目构建配置（自动处理编译、链接、资源拷贝）
├─ .gitignore          # Git 忽略规则（过滤编译产物与临时文件）
└─ README.md           # 项目说明

这个cmakelists用的是mingw64的编译器，如果你们的编译器是照着tut里给的话就可以用了，不是的话去下载mingw的15.2.0版本的。

我在cmakelists里基本上能让大家从仓库里拉下来就能用了，不需要自己再下载第三方库了。

大家引用图片等资源时记得要用相对路径且记得把“EchoGidge\assets\audio\test.wav”的“EchoGidge\”去掉，例如引用test.wav时，用"assets\audio\test.wav"。

上传的SDL库包含了基本的SDL和SDL_image（处理图片的）和SDL_mixer(处理音乐的)，如果还有需要的库再说。

记得到时候同步github仓库时不要直接再main branch里改，先自己创建一个新的branch，在你创建的branch里改，确定没问题了再PR到main branch。

这一版的所有结构都只是Windows的，如果用mac的话先不要用这版。
