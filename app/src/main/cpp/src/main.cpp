#include <algorithm>

#include "imgui.h"
#include "backends/imgui_impl_glfm.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFM/glfm.h>
#define FILE_COMPAT_ANDROID_ACTIVITY glfmAndroidGetActivity()
#include "file_compat.h"

#include <math.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "mud_library", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "mud_library", __VA_ARGS__))

#include <filesystem>

namespace fs = std::filesystem;

extern "C" {

struct HostNameFilter
{
    static int FilterInvalidHostname(ImGuiInputTextCallbackData* data)
    {
        if (data->EventChar < 256 && ':' == static_cast<char>(data->EventChar))
        {
            return 1;
        }
        return 0;
    }
};

static std::string getAssetDirectory()
{
    char fullPath[PATH_MAX];
    int rc = fc_resdir(fullPath, sizeof(fullPath));
    if (rc == -1)
    {
        LOGI("Failed to get res dir\n");
    }

    return fullPath;
}

void printFolder()
{
    fs::path cd = fs::current_path();
    try
    {
        for(auto entry : std::filesystem::directory_iterator(cd))
        {
            if (entry.is_directory() || entry.is_regular_file())
            {
                LOGI("%s\n", entry.path().generic_string().c_str());
            }
        }
    }
    catch(fs::filesystem_error& ex)
    {
        LOGI("filesystem error: %s\n", ex.what());
    }
}

ImFont* loadFont(ImGuiIO& io, const char* fontFile, float fontSize)
{
    if (FILE* fptest = fopen(fontFile, "rb"); fptest)
    {
        fseek(fptest, 0, SEEK_END);
        size_t sz = ftell(fptest);
        fseek(fptest, 0, SEEK_SET);
        char* data = static_cast<char*>(malloc(sz));
        fread(data, 1, sz, fptest);
        fclose(fptest);
        return io.Fonts->AddFontFromMemoryTTF(data, sz, fontSize);
    }
    return nullptr;
}

void android_main(struct android_app* app)
{
    glfmInitHint(GLFW_APP_DATA, app);

    if (!glfmInit())
    {
        return;
    }

    glfmDisplayHint(GLFM_CONTEXT_RENDER_API, GLFMRenderingAPIOpenGLES3);

    auto display = glfmCreateDisplay();

   // static const std::string assetDir = getAssetDirectory();

    //std::string fontFile = assetDir + "/Roboto-Medium.ttf";


    //printFolder();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

   ImFont* defaultFont = loadFont(io, "Roboto-Medium.ttf", 32.0f);
   ImFont* monoFont = loadFont(io, "RobotoMono-Medium.ttf", 32.0f);
    if (defaultFont)
    {
        io.FontDefault = defaultFont;
    }
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfm_InitForOpenGL(display, true);
    //we need initial events (to get the EGL context created
    glfmPollEvents();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    bool show_another_window = false;
    bool show_connect = true;
    bool connect_to_server = false;

    char server[256] = {0};
    int server_port = 3000;

    ImGuiStyle& style = ImGui::GetStyle();
    //style.ScaleAllSizes(2.0f);
    style.TouchExtraPadding = ImVec2(10.0f, 10.0f);

    double safeTop, safeRight, safeBottom, safeLeft;
    glfmGetDisplayChromeInsets(display, &safeTop, &safeRight, &safeBottom, &safeLeft);

    style.DisplaySafeAreaPadding.y = (float)safeTop;

    glfmSetKeyboardVisible(display, true);


    while(!glfmAppShouldClose())
    {
        glfmPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfm_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Connect", nullptr, &show_connect))
                {
                    memset(server, 0, 256);
                    strcpy(server, "127.0.0.1");
                    server_port = 4010;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (show_connect)
        {

            ImGui::SetNextWindowPos(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, -1.f), ImVec2(INFINITY, -1.f));
            ImGui::Begin(fmt::format("Connect to server {}", "YOH").c_str(), &show_connect);

            bool reclaimFocus = false;

            if (ImGui::InputText("Host", server, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter, HostNameFilter::FilterInvalidHostname))
            {
                //writeText("pressed enter on host");
                if (strlen(server) > 0)
                {
                    show_connect = false;
                    connect_to_server = true;
                }
                else
                {
                    reclaimFocus = true;
                }
            }

            ImGui::SetItemDefaultFocus();
            if (reclaimFocus)
            {
                ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
            }

            ImGui::InputInt("Port", &server_port);

            if (ImGui::Button("Connect"))
            {
                show_connect = false;
                connect_to_server = true;
            }

            if (connect_to_server)
            {
                //this is where we spawn new session t
                //auto pSession = std::make_unique<session_t>(fmt::format("{}:{}", server, server_port));
                //writeText(fmt::format("Connecting to {}", pSession->getSessionName()));

                //if (pSession->connect(server, server_port))
                {
                    //m_SessionThreads.emplace_back(startSession, pSession.get(), server, server_port, &m_ConsoleSessions);
                    //m_ConsoleSessions.add(std::move(pSession));
                   // writeText("Connected");
                }

                connect_to_server = false;
            }
            ImGui::End();
        }

        ImGuiWindowFlags outputFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysVerticalScrollbar;
        if (ImGui::Begin("Output", nullptr, outputFlags))
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (ImGui::Begin("Hello, world!"))
        {
            ImGui::PushFont(monoFont);
            ImGui::Text("This is a TEST WINDOW!");
             ImGui::Text("This is a TEST WINDOW!");
            ImGui::Text("This is a TEST WINDOW!");
            ImGui::PopFont();
            ImGui::End();
        }

        static bool show_demo_window = true;
        if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();
        int display_w, display_h;
        glfmGetDisplaySize(display, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfmSwapBuffers(display);

        if (io.WantTextInput)
        {
            glfmSetKeyboardVisible(display, true);
        }
    }
}

}
