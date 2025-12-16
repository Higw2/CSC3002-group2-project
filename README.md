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
mingw64 编译器（推荐 15.2.0 版本）<br>
CMake（3.10 或更高版本）<br>
Git（版本控制工具）<br>

如不满足前置需求，请安装必要的开发工具：
1. [MinGW-w64](https://www.mingw-w64.org/) (选择 x86_64 架构)
2. [CMake](https://cmake.org/download/) (3.15+)
3. [Git](https://git-scm.com/)

项目结构：
EchoGidge<br>
├─ assets          # 资源文件夹<br>
│  ├─ animations   # 动画资源<br>
│  ├─ fonts        # 字体资源<br>
│  ├─ maps         # 地图资源<br>
│  ├─ menu         # 菜单资源<br>
│  ├─ sounds       # 音效资源<br>
│  └─ sprites      # 精灵/贴图资源<br>
├─ src             # 源代码文件夹<br>
│  ├─ AudioManager.cpp/.h    # 音频管理类<br>
│  ├─ Camera.h               # 相机类（头文件）<br>
│  ├─ Coin.cpp/.h            # 金币类<br>
│  ├─ Game.cpp/.h            # 游戏核心类<br>
│  ├─ main.cpp               # 程序入口<br>
│  ├─ Player.cpp/.h          # 玩家类<br>
│  ├─ StartMenu.cpp/.h       # 开始菜单类<br>
│  ├─ TiledMap.cpp/.h        # Tiled地图类<br>
│  └─ VideoPlayer.cpp/.h     # 视频播放类<br>
└─ third-party     # 第三方依赖库<br>
   &emsp;├─ macos        # macOS平台依赖<br>
   &emsp;└─ windows      # Windows平台依赖<br>
      &emsp;&emsp;├─ glm               # 数学库<br>
      &emsp;&emsp;├─ nlohmann_json     # JSON解析库<br>
      &emsp;&emsp;├─ SDL2              # 基础图形/输入库<br>
      &emsp;&emsp;├─ SDL2_image        # 图片加载库<br>
      &emsp;&emsp;├─ SDL2_mixer        # 音频混音库<br>
      &emsp;&emsp;└─ SDL2_ttf          # 字体渲染库<br>
安装步骤：<br>
1. 克隆仓库：<br>
   git clone https://github.com/Higw2/CSC3002-group2-project.git<br>
   cd CSC3002-group2-project<br>
2. 使用 CMake 构建：<br>
   mkdir build<br>
   cd build<br>
   cmake --build .<br>
   ./EchoGidge.exe<br>

贡献者<br>
CSC3002 课程第二小组成员<br>
⭐ 如果这个项目对你有帮助，请给个 Star！ ⭐
