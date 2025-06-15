cmake_minimum_required(VERSION 3.15.0)
project(age VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILE_FEATURES)
    list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_20 HAS_CPP20)
    if(HAS_CPP20 GREATER -1)
        message(STATUS "Compiler supports C++20.")
    else()
        message(WARNING "Compiler does not support C++20.")
    endif()
else()
    message(WARNING "CMAKE_CXX_COMPILE_FEATURES is not available.")
endif()

# Enable cmake profiling:

set(ENGINE_NAME age)
set(PROFILE_BUILD OFF)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)
set(ENABLE_TESTING OFF)
# set(CMAKE_UNITY_BUILD ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(ENABLE_TESTING)
	enable_testing()
	add_subdirectory(external/googletest)
	add_subdirectory(tests)
endif()

add_subdirectory(platform)
add_subdirectory(engine)
# Only add the game you want to build here:
# add_subdirectory(games/asteroids)
# add_subdirectory(games/breakout)
add_subdirectory(games/gemtd)

