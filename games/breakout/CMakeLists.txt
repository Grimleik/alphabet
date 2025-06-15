
add_library(breakout SHARED
    breakout.cpp
)

target_precompile_headers(breakout PRIVATE ${CMAKE_SOURCE_DIR}/global_pch.h)
target_compile_definitions(breakout PRIVATE GAME_DLL_EXPORT)

if(MSVC AND PROFILE_BUILD)
    target_compile_options(breakout PRIVATE /Bt)
endif()
# target_compile_options(breakout PRIVATE /GR)

target_include_directories(breakout PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/include
)

add_custom_command(
    TARGET breakout
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "locked" > "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/breakout.pdb"
    COMMAND echo "Starting build"
)


add_custom_command(
    TARGET breakout
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
)

target_link_libraries(breakout PRIVATE ${ENGINE_NAME})