cmake_minimum_required(VERSION 3.22)

include(FetchContent)
FetchContent_Declare(
    GLFM
    GIT_REPOSITORY https://github.com/maztheman/glfm.git
    GIT_TAG v9
)
FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/maztheman/imgui.git
    GIT_TAG glfm-v5
)

FetchContent_MakeAvailable(GLFM ImGui)

project(mud_project C CXX)

add_subdirectory(src)