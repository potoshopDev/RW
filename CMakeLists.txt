cmake_minimum_required(VERSION 3.26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)

project(RW VERSION 1.0.0 DESCRIPTION "First example using" LANGUAGES CXX)


file(GLOB_RECURSE sources "${PROJECT_SOURCE_DIR}/src/*.cpp"
"${PROJECT_SOURCE_DIR}/src/*.h" )

include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

add_executable(RW WIN32 main.cpp ${sources})