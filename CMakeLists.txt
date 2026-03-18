cmake_minimum_required(VERSION 3.15)

SET(CMP0048 NEW)

project(RAYTRACER 
        VERSION 0.0.0
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(external/CMakeLists.txt)

set(RAYTRACER_SOURCE
src/main.cpp
)

# set(RAYTRACER_SOURCE_HEADERS
# )

add_executable(raytracer)
target_sources(raytracer PRIVATE
    ${RAYTRACER_SOURCE}
)