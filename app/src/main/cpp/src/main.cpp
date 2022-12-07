#include <algorithm>

#include "imgui.h"
#include "backends/imgui_impl_glfm.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFM/glfm.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "mud_library", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "mud_library", __VA_ARGS__))

extern "C" {

void android_main(struct android_app* app)
{
    LOGI("Pre Init hint\n");
    glfmInitHint(GLFW_APP_DATA, app);

    LOGI("Pre Init\n");
    if (!glfmInit())
    {
        return;
    }

    LOGI("Pre Display Hint\n");
    glfmDisplayHint(GLFM_CONTEXT_RENDER_API, GLFMRenderingAPIOpenGLES3);

    LOGI("Pre Create Display\n");
    auto display = glfmCreateDisplay();

    LOGI("Pre Setup Imgui\n");
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls


    LOGI("Pre style\n");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    LOGI("Pre ImGui_ImplGlfm_InitForOpenGL\n");
    ImGui_ImplGlfm_InitForOpenGL(display, true);
    glfmPollEvents();
    LOGI("Pre ImGui_ImplOpenGL3_Init\n");
    ImGui_ImplOpenGL3_Init("#version 300 es");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    LOGI("Pre Main loop\n");
    while(!glfmAppShouldClose())
    {
        glfmPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfm_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Hello, world!"))
        {
            ImGui::Text("This is a TEST WINDOW!");
             ImGui::Text("This is a TEST WINDOW!");
            ImGui::Text("This is a TEST WINDOW!");
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfmGetDisplaySize(display, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfmSwapBuffers(display);

    }
}

}
