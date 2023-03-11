
set(ENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_DIR "${ENGINE_ROOT_DIR}/src")
set(TOY_DIR "${ENGINE_ROOT_DIR}/toy3d")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/thirdparty")

file(GLOB_RECURSE HEADER_FILES "*.h" "*.hpp")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(toy3d STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(toy3d PUBLIC . ./thirdparty)
target_link_libraries(toy3d PUBLIC Vulkan::Vulkan)
target_compile_features(toy3d PUBLIC cxx_std_17)


