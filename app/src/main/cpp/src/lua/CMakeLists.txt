add_library(lualib STATIC)

#file(GLOB SRCS ${lua_SOURCE_DIR}/*.c)
set(SRCS ${lua_SOURCE_DIR}/onelua.c)

target_sources(lualib PRIVATE ${SRCS})
target_include_directories(lualib PUBLIC ${lua_SOURCE_DIR})


