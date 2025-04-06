if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("Compiling on Linux.")
    find_package(X11 REQUIRED)
    set(PLAT_INCLUDE_DIRS
        /usr/include/X11)
    set(PLATFORM_SOURCES
        # TODO Change to cpp.
        platform_linux.c
    )
    # target_link_options(age_plat PRIVATE -lX11 -lm)
    set(PLATFORM_LIBRARIES
        X11
        m)
    set(EXEC_NAME age_linux)
    # target_link_libraries(${EXEC_NAME} PRIVATE X11 m)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("Compiling on Windows.")

    set(PLATFORM_SOURCES
        platform_windows.cpp
    )
    set(EXEC_NAME age_win64)
    set(PLATFORM_LIBRARIES
        DbgHelp)
endif()

add_executable(${EXEC_NAME} main.cpp ${PLATFORM_SOURCES})
# add_executable(${EXEC_NAME} main.cpp platform_windows.cpp)
target_link_libraries(${EXEC_NAME} PRIVATE ${PLATFORM_LIBRARIES} ${ENGINE_NAME})
target_include_directories(${EXEC_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine/include
    ${PLAT_INCLUDE_DIRS})

if(MSVC AND PROFILE_BUILD)
    target_compile_options(${EXEC_NAME} PRIVATE /Bt)
endif()

target_precompile_headers(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/global_pch.h)