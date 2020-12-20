# This file is modified from that in the root of kendryte-standalone-sdk

# set this will supress some warnings
set(BUILDING_SDK "yes" CACHE INTERNAL "")

# basic config
cmake_minimum_required(VERSION 3.0)
include(./lib/kendryte-standalone-sdk/cmake/common.cmake)
project(8050a-display C CXX ASM)

# config self use headers
include(./lib/kendryte-standalone-sdk/cmake/macros.internal.cmake)
header_directories(${SDK_ROOT}/lib)
header_directories(inc/)
# build library first
add_subdirectory(lib/kendryte-standalone-sdk/lib)

# compile project
FILE(GLOB_RECURSE PROJ_SRC
    "src/*.c"
    "src/*.s"
    "src/*.S"
    "src/*.cpp"
)
add_source_files(${PROJ_SRC})
include(./lib/kendryte-standalone-sdk/cmake/executable.cmake)
