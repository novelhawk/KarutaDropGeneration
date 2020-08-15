cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(KarutaDropGenerator VERSION 1.0)

# set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

# Enable C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Executable drop
add_executable(drop src/main.cpp)

# Dependencies
if (NOT DEFINED CMAKE_WINDOWS)
    find_package(PNG REQUIRED)
    include_directories(PNG_INCLUDE_DIRS)
    target_link_libraries(drop LINK_PUBLIC ${PNG_LIBRARIES})

    find_package(WEBP REQUIRED)
    include_directories(WEBP_INCLUDE_DIRS)
    target_link_libraries(drop LINK_PUBLIC ${WEBP_LIBRARIES})
else()
    include_directories(./libpng/include/libpng12)
    target_link_libraries(drop LINK_PUBLIC ./libpng/lib/libpng.lib)

    include_directories(./libwebp/include/webp)
    target_link_libraries(./libwebp/lib/libwebp.lib)
endif()
