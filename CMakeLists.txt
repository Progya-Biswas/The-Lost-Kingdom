cmake_minimum_required(VERSION 3.16)
project(TheLostKingdom VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# raylib is fetched and built with the project on the first CMake configure.
# Pin the version so every developer and CI build uses the same graphics API.
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# C11 utility layer: parser, score helper, and dynamic string copy.
add_library(game_utils STATIC c_modules/c_utils.c)
target_include_directories(game_utils PUBLIC include)

# Existing object-oriented C++ engine is retained as a terminal fallback.
add_library(game_engine STATIC cpp_modules/Game.cpp)
target_include_directories(game_engine PUBLIC include)
target_link_libraries(game_engine PUBLIC game_utils)

# Main application: graphical top-down adventure powered by raylib.
add_executable(lost_kingdom
    main.cpp
    visual/VisualGame.cpp
)
target_include_directories(lost_kingdom PRIVATE include visual)
target_link_libraries(lost_kingdom PRIVATE raylib game_utils)

# Optional command-line version, retained for studying the C/C++ parser.
add_executable(lost_kingdom_cli cli_main.cpp)
target_include_directories(lost_kingdom_cli PRIVATE include)
target_link_libraries(lost_kingdom_cli PRIVATE game_engine game_utils)

if(MSVC)
    target_compile_options(game_utils PRIVATE /W4)
    target_compile_options(game_engine PRIVATE /W4)
    target_compile_options(lost_kingdom PRIVATE /W4)
    target_compile_options(lost_kingdom_cli PRIVATE /W4)
else()
    target_compile_options(game_utils PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(game_engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lost_kingdom PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lost_kingdom_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_command_parser tests/test_command_parser.c)
    target_link_libraries(test_command_parser PRIVATE game_utils)
    add_test(NAME command_parser COMMAND test_command_parser)
endif()
