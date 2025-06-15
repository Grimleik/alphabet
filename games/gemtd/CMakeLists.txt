
add_library(gemtd SHARED
    gemtd.cpp
)

target_precompile_headers(gemtd PRIVATE ${CMAKE_SOURCE_DIR}/global_pch.h)
target_compile_definitions(gemtd PRIVATE GAME_DLL_EXPORT)

if(MSVC AND PROFILE_BUILD)
    target_compile_options(gemtd PRIVATE /Bt)
endif()

target_include_directories(gemtd PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/include
)

add_custom_command(
    TARGET gemtd
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "locked" > "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/gemtd.pdb"
    COMMAND echo "Starting build"
)


add_custom_command(
    TARGET gemtd
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
)

target_link_libraries(gemtd PRIVATE ${ENGINE_NAME})