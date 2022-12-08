cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(
    GLFM
    GIT_REPOSITORY https://github.com/maztheman/glfm.git
    GIT_TAG v11
)
FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/maztheman/imgui.git
    GIT_TAG glfm-v6.2
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
FetchContent_MakeAvailable(GLFM ImGui yaml-cpp fmtlib sol)

project(mud_project C CXX)

add_subdirectory(src)