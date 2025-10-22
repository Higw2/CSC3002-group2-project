cmake_minimum_required(VERSION 3.15)
project(EchoGidge CXX)

# 配置 C++ 标准和编译选项（MinGW64 兼容）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")  # 开启警告和优化
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")  # 减少运行时依赖

# --------------------------
# 1. 第三方库路径（固定为 Windows 版）
# --------------------------
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/third-party/windows/SDL2")
set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/third-party/windows/SDL2_mixer")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/third-party/windows/SDL2_image")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/third-party/windows/glm")  # GLM 数学库路径
set(NLOHMANN_JSON_DIR "${CMAKE_SOURCE_DIR}/third-party/windows/nlohmann_json")  # JSON 库路径

# 头文件路径（项目自身 + 所有第三方库）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    # SDL 系列库
    ${SDL2_DIR}/x86_64-w64-mingw32/include
    ${SDL2_DIR}/x86_64-w64-mingw32/include/SDL2
    ${SDL2_MIXER_DIR}/x86_64-w64-mingw32/include
    ${SDL2_MIXER_DIR}/x86_64-w64-mingw32/include/SDL2
    ${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/include
    ${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/include/SDL2
    # GLM 数学库（纯头文件，只需包含目录）
    ${GLM_DIR}/include
    # nlohmann/json 库（纯头文件）
    ${NLOHMANN_JSON_DIR}/include
)

# 库文件路径（MinGW64 对应的 lib 目录）
link_directories(
    ${SDL2_DIR}/x86_64-w64-mingw32/lib
    ${SDL2_MIXER_DIR}/x86_64-w64-mingw32/lib
    ${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/lib
)

# --------------------------
# 2. 收集源代码（src 目录下所有 .cpp 文件）
# --------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")
message("找到的.cpp文件: ${SOURCES}")  # 调试：确认源文件是否正确收集
message(STATUS "SDL2 路径：${SDL2_DIR}")
message(STATUS "GLM 路径：${GLM_DIR}")
message(STATUS "JSON 路径：${NLOHMANN_JSON_DIR}")

# --------------------------
# 3. 生成可执行文件
# --------------------------
add_executable(EchoGidge ${SOURCES})  # 使用所有收集的源文件

# --------------------------
# 4. 链接库（MinGW 需按特定顺序链接）
# --------------------------
target_link_libraries(EchoGidge
    mingw32          # MinGW 运行时
    SDL2main         # SDL 主函数封装（必须在 SDL2 前）
    SDL2             # SDL2 核心库
    SDL2_mixer       # 音频库
    SDL2_image       # 图片库（支持 PNG 等格式）
    # 系统依赖库（窗口、图形、音频等）
    m user32 gdi32 winmm dxguid
)

# --------------------------
# 5. 自动拷贝运行时 DLL 和资源文件到编译目录
# --------------------------
add_custom_command(TARGET EchoGidge POST_BUILD
    # 拷贝 SDL2 主库 DLL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DIR}/x86_64-w64-mingw32/bin/SDL2.dll"
    "$<TARGET_FILE_DIR:EchoGidge>"

    # 拷贝 SDL2_mixer DLL 及音频解码依赖
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_MIXER_DIR}/x86_64-w64-mingw32/bin/SDL2_mixer.dll"
    #"${SDL2_MIXER_DIR}/x86_64-w64-mingw32/bin/libvorbis-0.dll"    # OGG 音频支持
    #"${SDL2_MIXER_DIR}/x86_64-w64-mingw32/bin/libvorbisfile-3.dll"
    #"${SDL2_MIXER_DIR}/x86_64-w64-mingw32/bin/libmodplug-1.dll"   # MOD 音频支持
    "$<TARGET_FILE_DIR:EchoGidge>"

    # 拷贝 SDL2_image DLL 及图像解码依赖
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/bin/SDL2_image.dll"
    "${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/bin/libpng16-16.dll"  # PNG 支持
    "${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/bin/zlib1.dll"       # 压缩支持
    #"${SDL2_IMAGE_DIR}/x86_64-w64-mingw32/bin/libjpeg-9.dll"   # JPEG 支持（可选）
    "$<TARGET_FILE_DIR:EchoGidge>"

    # 拷贝 assets 资源目录到编译目录（确保游戏能读取地图、精灵等资源）
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:EchoGidge>/assets"
)