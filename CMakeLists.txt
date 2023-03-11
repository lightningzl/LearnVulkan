cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Toy3D
        LANGUAGES CXX
        DESCRIPTION "a toy 3d renderer made in vulkan")

include(cmake/FindVulkan.cmake)
include(cmake/FindSDL2.cmake)
include(cmake/copydll.cmake)

find_program(GLSLC_PROGRAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_SOURCE_DIR}/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_SOURCE_DIR}/frag.spv)

add_subdirectory(engine)
add_subdirectory(sandbox)
