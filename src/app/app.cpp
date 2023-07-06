#include "ImguiRenderer.hpp"
#include "glad/glad.h"
#include <SDL2/SDL.h>
#include "widgets/FileDialog.hpp"
#include "src/decoder/audio_demux_decode.hpp"
#include "src/decoder/video_reader.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>


#include <map>

static const std::map<std::string, int> ffmpegToSDLAudioFmtMap = {
    {"s16be", AUDIO_S16MSB},
    {"s16le", AUDIO_S16LSB},
    {"s32be", AUDIO_S32MSB},
    {"s32le", AUDIO_S32LSB},
    {"f32be", AUDIO_F32MSB},
    {"f32le", AUDIO_F32LSB},
};

int main(int argv, char **args)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create Window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    auto window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, window_flags);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) // tie window context to glad's opengl funcs
    {
        return -1;
    }

    ImguiRenderer myimgui{window, glsl_version};

    Widgets::FileDialog fileDialog{"Pick a video file"};

    bool done = false;

    SDL_AudioSpec audio_spec; // the specs of our piece of music

    SDL_zero(audio_spec);
    

    VideoReader *vr = nullptr;

    GLuint tex_handle;
    glGenTextures(1, &tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        myimgui.NewFrame();
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    std::cout << "open File Dialog" << std::endl;
                    fileDialog.openDialog();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        // Render filedialog and then check if file selected
        if (fileDialog.draw())
        {
            const char *filePath = (char *)fileDialog.selected()[0].c_str();
            printf("Received Path\n %s", filePath);
            auto adec = new AudioDecoder();
            adec->demuxDecode(filePath, "./audio.raw");

            //Check if audio format is supported
            const std::string& formatStr = adec->getFormat();
            auto it = ffmpegToSDLAudioFmtMap.find(formatStr);
            if(it != ffmpegToSDLAudioFmtMap.end()) {
                int format = (*it).second;
                int numOfChannels = adec->getNumChannels();
                int sampleRate = adec->getSampleRate();

                std::cout << formatStr << std::endl;
                std::cout << sampleRate << std::endl;
                std::cout << numOfChannels << std::endl;
                int frame_size = 4;
                if(formatStr.starts_with("s16"))
                    frame_size = 2;
                std::string path{"audio.raw"};
                std::ifstream f(path, std::ios::in | std::ios::binary);
                // Obtain the size of the file.
                const auto sz = std::filesystem::file_size(path);

                // Create a buffer.
                std::string audio_buffer(sz, '\0');

                // Read the whole file into the buffer.
                f.read(audio_buffer.data(), sz);

                int samples = (sz / frame_size) * numOfChannels;
                
                audio_spec.samples = samples;
                audio_spec.freq = sampleRate;
                audio_spec.format = format;
                audio_spec.channels = numOfChannels;
                auto device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, nullptr, 0);
                if (NULL == device)
                {
                    std::cerr << "sound device error: " << SDL_GetError() << std::endl;
                    return -1;
                }
                int status = SDL_QueueAudio(device, audio_buffer.data(), sz);
                if (status < 0)
                {
                    std::cerr << "sound device error: " << SDL_GetError() << std::endl;
                    return -1;
                }
                // Play audio
                SDL_PauseAudioDevice(device, 0);
            }
            delete adec;
            vr = new VideoReader(filePath);
        }
        // Check if video file is selected
        if (vr != nullptr)
        {
            vr->video_reader_read_frame();
            const float w = vr->videoReaderState.width;
            const float h = vr->videoReaderState.height;
            glBindTexture(GL_TEXTURE_2D, tex_handle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, vr->videoReaderState.width, vr->videoReaderState.height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, vr->frame_buffer);
            ImGui::Begin("Video");
            ImGui::Image(reinterpret_cast<ImTextureID>(tex_handle), ImVec2{w, h});
            ImGui::End();
        }
        myimgui.Update();

        SDL_GL_SwapWindow(window);
    }
    myimgui.Shutdown();

    return 0;
}