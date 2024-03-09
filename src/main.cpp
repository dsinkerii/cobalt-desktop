/*

I have never ever used imgui nor have made any real serious c++ programs, i just 
graduated OOP C++, and all of this library, make, ifdef and whatnot
stuff scare the shit out of me but atleast i hope it works....
and i hope that i dont have to use chatgpt much here lol!

also that means im keeping all comments from the copypasted imgui tutorial, so bear with me on that one.....

- Dsinkerii

*/
#include <cstdlib>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <curl/curl.h>
#include <future>
#include <fstream> 

#include <json/json.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTexture(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}
void performGetRequest(std::string downloadURL = "https://www.youtube.com/watch?v=lDhaGag38XM", std::string instanceURL = "https://co.wuk.sh") {
    try 
	{
		curlpp::Cleanup cleaner;
		curlpp::Easy request;

		using namespace curlpp::Options;

		request.setOpt(Verbose(true));
		request.setOpt(Url(instanceURL + "/api/json"));
        std::string body = "{\"url\": \"" + downloadURL +"\"}";

        std::list<std::string> header; 
        header.push_back("Accept: application/json"); 
        header.push_back("Content-Type: application/json"); 

        request.setOpt(new curlpp::options::HttpHeader(header)); 

        request.setOpt(new curlpp::options::PostFields(body));
        request.setOpt(new curlpp::options::PostFieldSize(body.length()));
        //curlpp::Forms formParts;

        //formParts.push_back(new curlpp::FormParts::Content("url", downloadURL));
        //formParts.push_back(new curlpp::FormParts::Content("name2", "value2"));
        
        //request.setOpt(new curlpp::options::HttpPost(formParts)); 

        std::stringstream result;

        request.setOpt(cURLpp::Options::WriteStream(&result));

		request.perform();

        std::cout << request << std::endl;

        Json::Reader reader;
        Json::Value root;
        reader.parse(result.str(), root);

        std::cout << "\ndownload started.\n";

        curlpp::Easy requestFile;

        // Set the URL of the file to be downloaded
        std::string url = root["url"].asString().c_str();
        std::cout << url << "\n";

        // Set the URL to download from
        requestFile.setOpt(curlpp::Options::Url(url));

        // Open a file to write the downloaded data
        std::ofstream outputFile("../Downloads/bbb.mp4", std::ios::binary);

        // Set the write stream
        requestFile.setOpt(curlpp::Options::WriteStream(&outputFile));

        try {
            // Execute the request
            requestFile.perform();
            std::cout << "File downloaded successfully.\n";
        } catch (curlpp::RuntimeError& e) {
            std::cerr << "Error (runtime): " << e.what() << std::endl;
        } catch (curlpp::LogicError& e) {
            std::cerr << "Error (logic): " << e.what() << std::endl;
        } catch (std::exception& e) {
            std::cerr << "Error (std): " << e.what() << std::endl;
        }

        outputFile.close();
	}
	catch ( curlpp::LogicError & e ) {
		std::cout << e.what() << std::endl;
	}
	catch ( curlpp::RuntimeError & e ) {
		std::cout << e.what() << std::endl;
	}
}

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "cobalt-desktop", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("../misc/Fonts/CONSOLAB.ttf", 18.0f);
    ImFont* Bigger = io.Fonts->AddFontFromFileTTF("../misc/Fonts/CONSOLAB.ttf", 36.0f);
    ImGuiStyle &style = ImGui::GetStyle();
    style.FrameRounding = 10.0f;
    style.ChildRounding = 10.0f;
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    static constexpr ImWchar ranges[] = { 0x1F60, 0x1F64, 0 };
    static ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 1;
    cfg.MergeMode = true;
    cfg.FontBuilderFlags |= 1 << 8;
    io.Fonts->AddFontFromFileTTF("../misc/Fonts/seguiemj.ttf", 16.0f, &cfg, ranges); // unused lmao!!
    //IM_ASSERT(font != nullptr);

    // Our state
    static char inputBuffer[256] = "";
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    int my_image_width = 0;
    int my_image_height = 0;
    GLuint textureID = 0;
    GLuint textureIDVideo = 0;
    GLuint textureIDAudio = 0;
    GLuint textureIDVideoSelected = 0;
    GLuint textureIDAudioSelected = 0;
    GLuint textureIDSettings = 0;
    GLuint textureIDSeparator = 0;
    LoadTexture("../misc/Images/clipboard.png", &textureID, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/video.png", &textureIDVideo, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/audio.png", &textureIDAudio, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/video_selected.png", &textureIDVideoSelected, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/audio_selected.png", &textureIDAudioSelected, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/settings.png", &textureIDSettings, &my_image_width, &my_image_height);
    LoadTexture("../misc/Images/separator.png", &textureIDSeparator, &my_image_width, &my_image_height);
    
    int cobaltbrack_w = 0;
    int cobaltbrack_h = 0;
    GLuint textureID_bg = 0;
    GLuint textureID_bg2 = 0;
    LoadTexture("../misc/Images/cobalttrianglesorwhatever.png", &textureID_bg, &cobaltbrack_w, &cobaltbrack_h);
    LoadTexture("../misc/Images/cobalttrianglesorwhatever2.png", &textureID_bg2, &cobaltbrack_w, &cobaltbrack_h);

    enum audvidtogg {NONE, VIDEO, AUDIO};
    short vidAudToggle = 0;

    int xOffsetEvenBG = 0;
    bool showSettings = false;

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //cobalt >> animation
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("cobalt-bg", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

        ImGui::SetCursorPosY(0);

        int timesCanFitInX = (int)(ImGui::GetWindowWidth() / 84.0f+2);
        int timesCanFitInY = (int)(ImGui::GetWindowHeight() / 84.0f+2);

        for(int y = 0; y < timesCanFitInY; y++){
            for(int x = 0; x < timesCanFitInX; x++){
                ImGui::SetCursorPosX(x*84+(xOffsetEvenBG/4)-84);
                ImGui::SetCursorPosY(y*84*2);
                ImGui::Image((void*)(intptr_t)textureID_bg,ImVec2( 84.0f,84.0f ));
                ImGui::SameLine();
            }
        }
        for(int y = 0; y < timesCanFitInY; y++){
            for(int x = 0; x < timesCanFitInX; x++){
                ImGui::SetCursorPosX(x*84-(xOffsetEvenBG/4)-84);
                ImGui::SetCursorPosY(y*84*2+84);
                ImGui::Image((void*)(intptr_t)textureID_bg2,ImVec2( 84.0f,84.0f ));
                ImGui::SameLine();
            }
        }
        xOffsetEvenBG = (xOffsetEvenBG + 1) % (84*2);

        ImGui::End();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.


            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("cobalt-main-window", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);                          // Create a window called "Hello, world!" and append into it.

            ImGui::SetCursorPosX(10);
            ImGui::SetCursorPosY(10);
            if(ImGui::ImageButton((void*)(intptr_t)textureIDSettings, ImVec2(100, 25))){
                showSettings = !showSettings;
            }

            float windowWidth = ImGui::GetWindowWidth();
            float windowHeight = ImGui::GetWindowHeight();
            float textWidth = ImGui::CalcTextSize("cobalt").x;

            // Center the text horizontally
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::SetCursorPosY(windowHeight*0.5f-ImGui::CalcTextSize("cobalt").y);

            ImGui::Text("cobalt\n ");


            ImGui::SetCursorPosX((windowWidth) * 0.25f);
            ImGui::SetNextItemWidth(windowWidth*0.5f-ImGui::CalcTextSize(">>").x*4.5f);
            ImGui::InputText(" ", inputBuffer, IM_ARRAYSIZE(inputBuffer));

            ImGui::SameLine();
            
            ImGui::SetCursorPosX((windowWidth) * 0.75f-ImGui::CalcTextSize(">>").x*4);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()-10);

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(44,44,44,255));
            ImGui::PushFont(Bigger);
            ImGuiStyle* style = &ImGui::GetStyle();
            style->Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            bool SendPressed = ImGui::Button(">>");

            if(SendPressed){
                std::future<void> f = std::async(std::launch::async, performGetRequest,std::string(inputBuffer), "https://co.wuk.sh");
            }

            style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.10f, 0.10f, 0.40f);
            ImGui::PopStyleColor();
            ImGui::PopFont();
            

            ImGui::SetCursorPosX((windowWidth) * 0.25f);
            style->Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(200,200,200,255));
            bool Clipboard = ImGui::ImageButton((void*)(intptr_t)textureID, ImVec2(100, 25));

            ImGui::SameLine();

            //ImGui::Image((void*)(intptr_t)textureIDSeparator,ImVec2( 6,25 ));
            //ImGui::SetCursorPosX(2);

            //ImGui::SameLine();
            ImGui::SetCursorPosX(windowWidth*0.75f-260);
            bool Audio;
            bool Video;
            if(vidAudToggle == VIDEO){
                style->Colors[ImGuiCol_Button] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
                style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
                Video = ImGui::ImageButton((void*)(intptr_t)textureIDVideoSelected, ImVec2(100, 25));
            }else{
                style->Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
                style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.40f);
                Video = ImGui::ImageButton((void*)(intptr_t)textureIDVideo, ImVec2(100, 25));
            }

            ImGui::SameLine();
            if(vidAudToggle == AUDIO){
                style->Colors[ImGuiCol_Button] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
                style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
                Audio = ImGui::ImageButton((void*)(intptr_t)textureIDAudioSelected, ImVec2(100, 25));
            }else{
                style->Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 0.40f);
                style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.40f);
                Audio = ImGui::ImageButton((void*)(intptr_t)textureIDAudio, ImVec2(100, 25));
            }

            if(Video){
                vidAudToggle = VIDEO;
            }else if(Audio){
                vidAudToggle = AUDIO;
            }

            style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.40f);
            style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.10f, 0.10f, 0.40f);
            ImGui::PopStyleColor();

            ImGui::SetCursorPosX(10);
            ImGui::SetCursorPosY(10);
            if(showSettings){
                ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(21,21,21,255));
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(88,88,88,255));
                ImGui::BeginChild("settings", ImVec2(windowWidth/2-20,windowHeight-20));
                ImGui::Text(" \n ver: P.O.C. DEMO");
                ImGui::SameLine();
                ImGui::SetCursorPosX(windowWidth/2-38);
                if(ImGui::Button("x")){
                    showSettings = !showSettings;
                }
                ImGui::SetCursorPosX(-15);
                ImGui::Image((void*)(intptr_t)textureIDSettings,ImVec2(200, 50));


                ImGui::PopStyleColor();
                ImGui::PopStyleColor();

                ImGui::SetCursorPosX(10);
                ImGui::BeginChild("settings-blackpart", ImVec2(windowWidth/2-40,windowHeight-120));
                ImGui::SetCursorPosX(10);
                ImGui::SetCursorPosY(10);
                ImGui::Text("no settings atm.... check in another day?");
                ImGui::EndChild();


                ImGui::EndChild();
            }
            ImGui::End();


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



