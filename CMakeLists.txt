cmake_minimum_required(VERSION 3.15)

SET(CMP0048 NEW)

project(RAYTRACER 
        VERSION 0.0.0
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(external)

set(RAYTRACER_SOURCES
    src/main.cpp
)

set(RAYTRACER_HEADERS
    src/vecmath/Bounds3.h
    src/vecmath/Point2.h
    src/vecmath/Point3.h
    src/vecmath/Vector2.h
    src/vecmath/Vector3.h

    src/light/ILightSource.h
    src/light/Spectrum.h
)

add_library(${PROJECT_NAME}_lib STATIC
    ${RAYTRACER_SOURCES}
    ${RAYTRACER_HEADERS}
)

target_link_libraries(${PROJECT_NAME}_lib PUBLIC OpenQMC::OpenQMC)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

enable_testing()
add_subdirectory(tests)