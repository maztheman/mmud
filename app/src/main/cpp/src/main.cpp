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

#include <core/session.hpp>
#include <core/console_session.h>


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

static void printFolder()
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

static ImFont* loadFont(ImGuiIO& io, const char* fontFile, float fontSize)
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

static void startSession(kms::session_t* session, std::string serverAddr, int port, kms::protected_vector_t<kms::session_t>* sessions)
{
    session->play();
    LOGI(fmt::format("session closed [{}:{}]\n", serverAddr, port).c_str());
    session->isDeleted() = true;
}

/*

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






*/

static void showConnect(
    ImFont* font,
    bool* pShowConnect,
    bool& connect_to_server,
    char (&server)[256],
    int& server_port,
    std::vector<std::thread>& m_SessionThreads,
    kms::protected_vector_t<kms::session_t>& m_ConsoleSessions)
{
    ImGui::PushFont(font);
    if (*pShowConnect)
    {
        ImGui::OpenPopup("##connect");
        //ImGui::SetNextWindowPos(ImVec2(400.0f, 200.0f), ImGuiCond_FirstUseEver);
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(600.f, -1.f), ImVec2(INFINITY, -1.f));
        if (ImGui::BeginPopupModal("##connect", pShowConnect))
        {
            bool reclaimFocus = false;
            if (ImGui::InputText("Host", server, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter, HostNameFilter::FilterInvalidHostname))
            {
                //writeText("pressed enter on host");
                if (strlen(server) > 0)
                {
                    *pShowConnect = false;
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
                *pShowConnect = false;
                connect_to_server = true;
                ImGui::CloseCurrentPopup();
            }

            if (connect_to_server)
            {
                //this is where we spawn new session t
                auto pSession = std::make_unique<kms::session_t>(fmt::format("{}:{}", server, server_port));
                //writeText(fmt::format("Connecting to {}", pSession->getSessionName()));

                if (pSession->connect(server, server_port))
                {
                    m_SessionThreads.emplace_back(startSession, pSession.get(), server, server_port, &m_ConsoleSessions);
                    m_ConsoleSessions.add(std::move(pSession));
                   // writeText("Connected");
                }

                connect_to_server = false;
            }
            ImGui::EndPopup();
        }
    }

    ImGui::PopFont();
}

//allow lua script or yaml to define the commands
//allow many pages

void drawPage1BottomBar(float unitSize, const float unitRowSize, bool& show_connect,
 char (&server)[256],
 int& server_port,
 int& page,
 kms::session_t* activeSession)
{
    if (ImGui::Button("Look", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("look");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("Exits", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("exits");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Button("test 3", ImVec2(unitSize * 2, unitRowSize));
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("North", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("north");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("FN", ImVec2(unitSize * 2, unitRowSize)))
    {
        page = 2;
    }
    //line 2
    if (ImGui::Button("Scan", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("scan");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Button("test 2", ImVec2(unitSize * 2, unitRowSize));
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("West", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("west");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("South", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("south");
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("East", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->addInput("east");
        }
    }
}

void drawPage2BottomBar(float unitSize, const float unitRowSize, bool& show_connect,
 char (&server)[256],
 int& server_port,
 int& page,
 kms::session_t* activeSession)
{
    if (ImGui::Button("NewCon", ImVec2(unitSize * 2, unitRowSize)))
    {
        show_connect = true;
        memset(server, 0, 256);
        strcpy(server, "127.0.0.1");
        server_port = 4010;
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("D/C", ImVec2(unitSize * 2, unitRowSize)))
    {
        if (activeSession != nullptr)
        {
            activeSession->close();
        }
    }
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Button("test 3", ImVec2(unitSize * 2, unitRowSize));
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("empty", ImVec2(unitSize * 2, unitRowSize)))
    {
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("<-", ImVec2(unitSize * 2, unitRowSize)))
    {
        page = 1;
    }
    //line 2
    if (ImGui::Button("empty", ImVec2(unitSize * 2, unitRowSize)))
    {
    }
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::Button("test 2", ImVec2(unitSize * 2, unitRowSize));
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("empty", ImVec2(unitSize * 2, unitRowSize)))
    {
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("empty", ImVec2(unitSize * 2, unitRowSize)))
    {
    }
    ImGui::SameLine(0.0f, 0.0f);
    if (ImGui::Button("empty", ImVec2(unitSize * 2, unitRowSize)))
    {
    }
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
    using ro_strings = kms::base_protected_vector_t<std::string>::READ_ONLY_VECTOR;

    //printFolder();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

   ImFont* defaultFont = loadFont(io, "Roboto-Medium.ttf", 32.0f);
   ImFont* monoFont = loadFont(io, "RobotoMono-Medium.ttf", 32.0f);
   ImFont* monoFontTwice = loadFont(io, "RobotoMono-Medium.ttf", 64.0f);
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
    bool show_connect = false;
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

    static std::vector<std::thread> m_SessionThreads;
    static kms::protected_vector_t<kms::session_t> m_ConsoleSessions;

    const float unitRowSize = 75.0f;
    const float totalSize = 3 * unitRowSize;
    std::array<char, 256> inputBuffer;
    int page = 1;


    while(!glfmAppShouldClose())
    {
        kms::session_t* activeSession = nullptr;
        glfmPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfm_NewFrame();
        ImGui::NewFrame();

        glfmGetDisplayChromeInsets(display, &safeTop, &safeRight, &safeBottom, &safeLeft);

        auto sz = ImGui::CalcTextSize("01234567890123456789012345678901234567890123456789012345678901234567890123456789");

         int display_w, display_h;
        glfmGetDisplaySize(display, &display_w, &display_h);


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowPos(ImVec2(0.0f, safeTop), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(display_w, display_h - safeTop - safeBottom - totalSize));
        if (ImGui::Begin("##main", nullptr,
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            if (ImGui::BeginTabBar("##tabby"))
            {
                uint32_t tab = 0, sessionNum = 0;
                ImGui::PushFont(monoFont);
                for(std::vector<kms::session_t*> consoleSessions; auto* cs : m_ConsoleSessions.get(consoleSessions))
                {
                    if (cs->isDeleted())
                    {
                        sessionNum++;
                        continue;
                    }

                    std::string item = fmt::format("[{}:{}]", cs->getSessionName(), tab++);

                    if (ImGui::BeginTabItem(item.c_str()))
                    //if (ImGui::Begin(fmt::format("##{:p}", static_cast<void*>(cs)).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
                    {
                        activeSession = cs;
                        if (ImGui::BeginChild(1U, ImVec2(sz.x, 30.0f * (ImGui::GetTextLineHeightWithSpacing())), false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysVerticalScrollbar))
                        {
                            for(ro_strings consoleText; const auto* line : cs->readText(consoleText))
                            {
                                if (!line->ends_with("\r\n"))
                                {
                                    if (!line->empty())
                                    {
                                        size_t szz = strlen(line->c_str());
                                        if (szz >= 2)
                                        {
                                            char c0 = *(line->begin() + szz - 2);
                                            char c1 = *(line->begin() + szz - 1);
                                            LOGI("[noctrl] [%02X%02X] %s ", c0, c1, line->c_str());
                                        }
                                    }
                                   // ImGui::SameLine();
                                }
                                ImGui::Text("%s", line->c_str());
                            }

                            if (cs->scrollToBottom() || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                            {
                                ImGui::SetScrollHereY(1.0f);
                                cs->scrollToBottom() = false;
                            }

                            ImGui::EndChild();
                        }
                        /*auto& inputBuf = cs->getInputBuffer();
                        ImGui::PushItemWidth(-1.0f);
                        if (ImGui::InputText("##mudinput", inputBuf.data(), inputBuf.size(), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            cs->addInput(inputBuf.data());
                        }
                        ImGui::PopItemWidth();*/
                        //ImGui::End();
                        ImGui::EndTabItem();
                    }
                    sessionNum++;
                }

                ImGui::PopFont();
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        ImGui::PopStyleVar(2);

       static bool show_demo_window = false;
        if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::SetNextWindowPos(ImVec2(0.0f, display_h - safeBottom - totalSize), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(display_w, totalSize));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        const bool openActionBar = ImGui::Begin("bottom menu", nullptr,
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        );

        if (openActionBar)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
            {
                ImGui::SetKeyboardFocusHere(0);
            }
            ImGui::PushFont(monoFontTwice);
            ImGui::PushItemWidth(-1.0f);
            if (ImGui::InputText("##mudinput_global", inputBuffer.data(), inputBuffer.size(), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
            {
                if (activeSession != nullptr)
                {
                    activeSession->addInput(inputBuffer.data());
                }
            }
            ImGui::PopItemWidth();
            ImGui::PopFont();

            if (page == 1)
            {
                drawPage1BottomBar(display_w / 10.0f, unitRowSize, show_connect, server, server_port, page, activeSession);
            }
            else
            {
                drawPage2BottomBar(display_w / 10.0f, unitRowSize, show_connect, server, server_port, page, activeSession);
            }

            ImGui::PopStyleVar();
        }
        ImGui::PopStyleVar(2);
        ImGui::End();

        showConnect(monoFontTwice, &show_connect, connect_to_server, server, server_port, m_SessionThreads, m_ConsoleSessions);

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfmSwapBuffers(display);

        if (io.WantTextInput)
        {
            if (!glfmIsKeyboardVisible(display))
            {
                glfmSetKeyboardVisible(display, true);
            }
        }
    }
}

}
