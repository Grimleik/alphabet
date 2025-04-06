
add_library(asteroids SHARED
    asteroids.cpp
)

target_precompile_headers(asteroids PRIVATE ${CMAKE_SOURCE_DIR}/global_pch.h)
target_compile_definitions(asteroids PRIVATE GAME_DLL_EXPORT)

if(MSVC AND PROFILE_BUILD)
    target_compile_options(asteroids PRIVATE /Bt)
endif()

target_include_directories(asteroids PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/include
)

# Hacky way to generate a new pdb on each invocation to trick the debugger
# during hot reloading:
# https://stackoverflow.com/questions/36387105/how-do-you-in-visual-studio-generate-a-pdb-file-with-a-random-outputted-name
# set(CUSTOM_PDB_NAME "$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString(\"HH_mm_ss_fff\")).pdb")
# target_link_options(asteroids PRIVATE "/PDB:${CUSTOM_PDB_NAME}")

add_custom_command(
    TARGET asteroids
    PRE_BUILD
    # COMMAND timeout /t 1 /nobreak
    COMMAND ${CMAKE_COMMAND} -E echo "locked" > "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/asteroids.pdb"
    # COMMAND powershell -Command "Start-Sleep -Seconds 2"
    COMMAND echo "Starting build"
)


add_custom_command(
    TARGET asteroids
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_BINARY_DIR}/bin/Debug/hotreloading"
)

target_link_libraries(asteroids PRIVATE ${ENGINE_NAME})

# add_custom_command(
#     TARGET asteroids
#     PRE_BUILD
#     COMMAND ${CMAKE_SOURCE_DIR}/misc/generate_pdb_name.bat > ${CMAKE_SOURCE_DIR}/misc/pdb_name.txt
#     file (READ "${CMAKE_SOURCE_DIR}/misc/pdb_name.txt" GENERATED_PDB_NAME)
#     set_target_properties (asteroids PROPERTIES
#          COMPILE_PDB_NAME ${GENERATED_PDB_NAME}
#     )
# )

# add_custom_command(
#     TARGET asteroids
#     PRE_BUILD
#     COMMAND ${CMAKE_SOURCE_DIR}/misc/remove_pdb.bat
# )

# set_target_properties(asteroids PROPERTIES
#     COMPILE_PDB_NAME "${GENERATED_PDB_NAME}"
#     COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/"
# )

# add_custom_command(TARGET asteroids POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different
#  $<TARGET_PDB_FILE:asteroids>
#  ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
# )
