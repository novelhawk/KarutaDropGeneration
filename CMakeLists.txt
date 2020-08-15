cmake_minimum_required(VERSION 3.18)

project(KarutaDropGenerator VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_BUILD_TYPE Debug)

add_definitions(-DPNG_TRANSFORM_IDENTITY)

include_directories(
    ${PROJECT_SOURCE_DIR}/src 
    ${PROJECT_SOURCE_DIR}/include)

add_executable(drop src/main.cpp)
target_link_libraries(drop webp png)
