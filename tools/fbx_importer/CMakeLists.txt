cmake_minimum_required(VERSION 3.10)
project(fbx_importer)

set(CMAKE_CXX_STANDARD 11)

find_package(assimp REQUIRED)

add_executable(fbx_importer main.cpp)

target_include_directories(fbx_importer PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/thirdparty/stb
)

target_link_libraries(fbx_importer PRIVATE 
    assimp
    )
