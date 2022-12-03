cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(
    GLFM
    GIT_REPOSITORY https://github.com/maztheman/glfm.git
    GIT_TAG v13
)
FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/maztheman/imgui.git
    GIT_TAG glfm-v7.4
)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
    GIT_TAG yaml-cpp-0.7.0
)
FetchContent_Declare(
    fmtlib
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 9.1.0
)
FetchContent_Declare(
    sol
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.4
)
FetchContent_MakeAvailable(GLFM ImGui yaml-cpp fmtlib sol lua)

project(mud_project C CXX)

set(CMAKE_CXX_STANDARD 20)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)



add_subdirectory(src)
