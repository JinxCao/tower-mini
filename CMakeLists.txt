cmake_minimum_required(VERSION 3.22)
project(tower_mini CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# 依赖（相对路径）
# -----------------------------
set(SDL2_DIR         "${CMAKE_SOURCE_DIR}/third_party/SDL2")
set(SDL2_IMAGE_DIR   "${CMAKE_SOURCE_DIR}/third_party/SDL2_image")
set(SDL2_TTF_DIR     "${CMAKE_SOURCE_DIR}/third_party/SDL2_ttf")
set(SDL2_MIXER_DIR   "${CMAKE_SOURCE_DIR}/third_party/SDL2_mixer")

# 头文件目录
include_directories(
    ${SDL2_DIR}/include
    ${SDL2_IMAGE_DIR}/include
    ${SDL2_TTF_DIR}/include
    ${SDL2_MIXER_DIR}/include
)

# 库目录
link_directories(
    ${SDL2_DIR}/lib/x64
    ${SDL2_IMAGE_DIR}/lib/x64
    ${SDL2_TTF_DIR}/lib/x64
    ${SDL2_MIXER_DIR}/lib/x64
)

# -----------------------------
# 源码
# -----------------------------
add_executable(tower_mini
    src/main.cpp
    src/game.cpp
    src/game.h
)

# MSVC 编译选项（中文源码不出警告）
if (MSVC)
    add_compile_options(/utf-8)
endif()

# -----------------------------
# 链接
# -----------------------------
target_link_libraries(tower_mini
    SDL2 SDL2main
    SDL2_image
    SDL2_ttf
    SDL2_mixer
)

# -----------------------------
# 构建后复制 DLL 和资源
# -----------------------------
set(RUNTIME_DLLS
    ${SDL2_DIR}/lib/x64/SDL2.dll

    ${SDL2_IMAGE_DIR}/lib/x64/SDL2_image.dll
    ${SDL2_IMAGE_DIR}/lib/x64/libpng16.dll
    ${SDL2_IMAGE_DIR}/lib/x64/zlib1.dll
    ${SDL2_IMAGE_DIR}/lib/x64/brotlicommon.dll
    ${SDL2_IMAGE_DIR}/lib/x64/brotlidec.dll
    ${SDL2_IMAGE_DIR}/lib/x64/bz2.dll

    ${SDL2_TTF_DIR}/lib/x64/SDL2_ttf.dll
    ${SDL2_TTF_DIR}/lib/x64/freetype.dll

    ${SDL2_MIXER_DIR}/lib/x64/SDL2_mixer.dll
    ${SDL2_MIXER_DIR}/lib/x64/libmpg123-0.dll
)

add_custom_command(TARGET tower_mini POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RUNTIME_DLLS} $<TARGET_FILE_DIR:tower_mini>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:tower_mini>/assets
)
