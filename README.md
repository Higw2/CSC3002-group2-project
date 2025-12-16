# CSC3002-group2-project
2D Platformer Game (C++ ; SDL2; Glm; Tiled)
本项目是一款基于现代 C++ 和 SDL2 框架开发的 2D 平台跳跃游戏。它集成了多个多媒体库用于图形渲染、音频处理和输入控制，并结合 Tiled 地图编辑器进行关卡设计，同时使用 glm 库实现相机管理功能。游戏支持 Windows 平台，配备预配置的构建工具以简化开发流程。

## 📋 系统要求：
- **操作系统**: Windows 10 或更高版本
- **编译器**: MinGW-w64 (x86_64-posix-seh 推荐)
- **CMake**: 版本 3.15 或更高
- **内存**: 最少 4GB RAM
- **显卡**: 支持 OpenGL 2.0 或更高

前置要求：
mingw64 编译器（推荐 15.2.0 版本）
CMake（3.10 或更高版本）
Git（版本控制工具）

如不满足前置需求，请安装必要的开发工具：
1. [MinGW-w64](https://www.mingw-w64.org/) (选择 x86_64 架构)
2. [CMake](https://cmake.org/download/) (3.15+)
3. [Git](https://git-scm.com/)

项目结构：
EchoGidge
├─ assets          # 资源文件夹
│  ├─ animations   # 动画资源
│  ├─ fonts        # 字体资源
│  ├─ maps         # 地图资源
│  ├─ menu         # 菜单资源
│  ├─ sounds       # 音效资源
│  └─ sprites      # 精灵/贴图资源
├─ src             # 源代码文件夹
│  ├─ AudioManager.cpp/.h    # 音频管理类
│  ├─ Camera.h               # 相机类（头文件）
│  ├─ Coin.cpp/.h            # 金币类
│  ├─ Game.cpp/.h            # 游戏核心类
│  ├─ main.cpp               # 程序入口
│  ├─ Player.cpp/.h          # 玩家类
│  ├─ StartMenu.cpp/.h       # 开始菜单类
│  ├─ TiledMap.cpp/.h        # Tiled地图类
│  └─ VideoPlayer.cpp/.h     # 视频播放类
└─ third-party     # 第三方依赖库
   ├─ macos        # macOS平台依赖
   └─ windows      # Windows平台依赖
      ├─ glm               # 数学库
      ├─ nlohmann_json     # JSON解析库
      ├─ SDL2              # 基础图形/输入库
      ├─ SDL2_image        # 图片加载库
      ├─ SDL2_mixer        # 音频混音库
      └─ SDL2_ttf          # 字体渲染库
安装步骤：
1. 克隆仓库：
   git clone https://github.com/Higw2/CSC3002-group2-project.git
   cd CSC3002-group2-project
2. 使用 CMake 构建：
   mkdir build
   cd build
   cmake --build .
   ./EchoGidge.exe

贡献者
CSC3002 课程第二小组成员
⭐ 如果这个项目对你有帮助，请给个 Star！ ⭐
