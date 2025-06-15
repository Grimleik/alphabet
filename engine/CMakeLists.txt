set(SHARED_SRC
    src/input.cpp
    src/renderer.cpp
    src/memorymanager.cpp
    src/entity.cpp
)
set(PLAT_SRC)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLAT_SRC
    #     src/renderer_opengl_linux.cpp
    #     src/renderer_software_linux.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLAT_SRC
        src/renderer_opengl_windows.cpp
        src/renderer_software_windows.cpp
    )
endif()

set(SRC
    ${SHARED_SRC}
    ${PLAT_SRC}
)

add_library(${ENGINE_NAME} STATIC
	${SRC}
)

# add_library(${ENGINE_NAME} STATIC
#     src/input.cpp
#     src/renderer.cpp
#     src/memorymanager.cpp
#     src/entity.cpp
# )

target_precompile_headers(${ENGINE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/global_pch.h)

target_include_directories(${ENGINE_NAME} PRIVATE
    ${PLAT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
)

# NOTE: Platform specific atm.
if(MSVC AND PROFILE_BUILD)
    target_compile_options(${ENGINE_NAME} PRIVATE /Bt)
endif()
