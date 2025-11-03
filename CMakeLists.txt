cmake_minimum_required(VERSION 3.22)
project(tower_mini CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain（你已有）
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "E:/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "E:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)

add_executable(tower_mini
  src/main.cpp
  src/game.cpp
)

# SDL2
target_link_libraries(tower_mini PRIVATE SDL2::SDL2main SDL2::SDL2)

# SDL2_ttf（兼容）
if(TARGET SDL2::SDL2_ttf)
  target_link_libraries(tower_mini PRIVATE SDL2::SDL2_ttf)
elseif(TARGET SDL2_ttf::SDL2_ttf)
  target_link_libraries(tower_mini PRIVATE SDL2_ttf::SDL2_ttf)
else()
  message(FATAL_ERROR "SDL2_ttf target not found after find_package(SDL2_ttf).")
endif()

# SDL2_image（兼容）
if (TARGET SDL2::SDL2_image)
  target_link_libraries(tower_mini PRIVATE SDL2::SDL2_image)
elseif(TARGET SDL2_image::SDL2_image)
  target_link_libraries(tower_mini PRIVATE SDL2_image::SDL2_image)
else()
  message(FATAL_ERROR "SDL2_image target not found after find_package(SDL2_image).")
endif()

if (MSVC)
  add_compile_options(/utf-8)
endif()
