add_library(backend STATIC)

file(GLOB_RECURSE SRCS *.cpp)

target_sources(backend PRIVATE ${SRCS})
target_include_directories(backend PUBLIC ".")
target_link_libraries(backend PUBLIC
    yaml-cpp fmt sol2 lualib
)
target_precompile_headers(backend PUBLIC <vector> <fmt/format.h> <compat/platform.h> pch.h)
target_compile_features(backend PUBLIC cxx_std_${CMAKE_CXX_STANDARD})
