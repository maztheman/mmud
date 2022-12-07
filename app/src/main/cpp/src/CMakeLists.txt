add_library(mud_library SHARED)

SET(SRCS
    main.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfm.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# hold off on including:
#     ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
#

target_sources(mud_library PRIVATE ${SRCS})
target_compile_definitions(mud_library PRIVATE IMGUI_IMPL_OPENGL_ES3 IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
target_include_directories(mud_library PRIVATE ${imgui_SOURCE_DIR} ${ANDROID_NDK}/sources/android/native_app_glue/)
target_link_libraries(mud_library PRIVATE glfm)