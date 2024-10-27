cmake_minimum_required(VERSION 3.30.0)

project(Game)

add_library(glad ${PROJECT_SOURCE_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(SDL2main STATIC IMPORTED)
set_target_properties(SDL2main PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/include/SDL2-2.30.5/lib/x64/SDL2main.lib)
target_include_directories(SDL2main INTERFACE ${PROJECT_SOURCE_DIR}/include/SDL2-2.30.5/include)

add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/include/SDL2-2.30.5/lib/x64/SDL2.lib)
target_include_directories(SDL2 INTERFACE ${PROJECT_SOURCE_DIR}/include/SDL2-2.30.5/include)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE src
    "src/main.cpp"
)

add_executable(main ${src})
set_target_properties(main PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

target_include_directories(main PRIVATE ${PROJECT_SOURCE_DIR}/include/glm)

target_link_libraries(main PRIVATE SDL2main SDL2 glad)
