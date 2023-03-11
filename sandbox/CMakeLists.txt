add_executable(sandbox)
aux_source_directory(./ SANDBOX_SRC)
target_sources(sandbox PRIVATE ${SANDBOX_SRC})
target_link_libraries(sandbox PUBLIC toy3d SDL2)
CopyDLL(sandbox)

target_include_directories(sandbox PUBLIC ${ENGINE_ROOT_DIR} ${THIRD_PARTY_DIR})
