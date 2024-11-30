#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <thread>
#include <vector>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <future>
#include <openssl/md5.h>
#include <time.h>
#include <unordered_map>
#include <ctime>
#include <cmath>
#include <sstream>  // Include this header for std::stringstream
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"

#include "./Core.h"
#include "./Sprite.h"
#include "./Common.h"
#include "./Camera.h"
#include "./Player.hpp"
#include "./Map.hpp"




class Game {
public:
    Game(bool enableImGui = false) : width(800), height(600), running(true), lastFrameTime(0), deltaTime(0.0f), enableImGui(enableImGui) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG;
        SDL_Init(SDL_INIT_VIDEO);
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            exit(1);
        }
        
        SDL_GLContext glContext = SDL_GL_CreateContext(window);
        if (glContext == nullptr) {
            std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (GLEW_OK != err) {
            std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        }

        // Initialize ImGui
        if (enableImGui) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
            ImGui_ImplSDLRenderer2_Init(renderer);

            // Customize colors
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_WindowBg] = ImVec4(245.0f / 255.0f, 245.0f /255.0f, 220.0f / 255.0f, 1.0f); // Background color
            //style.Colors[ImGuiCol_TitleBg] = ImVec4(245.0f / 255.0f, 245.0f/255.0f, 220.0f/255.0f, 1.0f); // Title bar background color
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(172.0f / 255.0f, 128.0f / 255.0f, 79.0f/255.0f, 1.0f); // Active title bar background color
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(245.0f/255.0f, 245.0f/255.0f, 220.0f/255.0f, 1.0f); // Collapsed title bar background color

            // Customize font
            style.Colors[ImGuiCol_Text] = ImVec4(175.0f/255.0f, 128.0f/255.0f, 79.0f/255.0f, 1.0f);

            // Set the desired font size here
            float fontSize = 20.0f; // Change this value to your desired font size
            ImFontConfig config;
            config.SizePixels = fontSize;

            io.Fonts->AddFontDefault(&config);
            io.Fonts->Build();

            // Customize window padding
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.WindowRounding = 5.0f; // Rounded corners
            style.WindowBorderSize = 1.0f; // Border size
        }



        map = new Map(renderer, MAP_WIDTH, MAP_HEIGHT, GRASS_TILE_SIZE);
        player = new Player(renderer, "../Assets/Character/s1.png");
        player2 = new Player(renderer, "../Assets/Character/s1.png"); // Create the second player
        camera = createCamera(width, height, MAP_WIDTH * GRASS_TILE_SIZE, MAP_HEIGHT * GRASS_TILE_SIZE);
        startTime = SDL_GetTicks();
        currentPhase = "Day";

        // Read the build hash from the file
        std::ifstream buildHashFile("build_hash.txt");
        if (buildHashFile.is_open()) {
            std::getline(buildHashFile, buildHash);
            buildHashFile.close();
        } else {
            buildHash = "Unknown";
        }
        SDL_ShowCursor(SDL_ENABLE);

        customCursor = createCustomCursor("../Assets/Cursors/back/back6.png", 0, 0);
        mouseDownCursor = createCustomCursor("../Assets/Cursors/back/back5.png", 0, 0);
        if (customCursor) {
            SDL_SetCursor(customCursor);
            SDL_Log("Custom cursor set successfully.");
        } else {
            SDL_Log("Failed to set custom cursor.");
        }
    }

    ~Game() {
        delete map;
        delete player;
        delete player2; // Clean up the second player
        free(camera);
        SDL_FreeCursor(customCursor);
        SDL_FreeCursor(mouseDownCursor);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        // Cleanup ImGui
        if (enableImGui) {
            ImGui_ImplSDLRenderer2_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        }

    }

    void run() {
        camera->update(camera, player->getX(), player->getY(), player->getWidth(), player->getHeight(), 0);
        while (running) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                width = e.window.data1;
                height = e.window.data2;
                SDL_RenderSetLogicalSize(renderer, width, height);
                camera->setViewportSize(camera, width, height);
                camera->update(camera, player->getX(), player->getY(), player->getWidth(), player->getHeight(), 0.0f); // Pass 0 for deltaTime initially
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        player->move(0, -5);
                        break;
                    case SDLK_DOWN:
                        player->move(0, 5);
                        break;
                    case SDLK_LEFT:
                        player->move(-5, 0);
                        break;
                    case SDLK_RIGHT:
                        player->move(5, 0);
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                    case SDLK_LEFT:
                    case SDLK_RIGHT:
                        player->setMoving(false);
                        break;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                if (e.wheel.y > 0) {
                    camera->adjustZoom(camera, 0.1f); // Zoom in
                } else if (e.wheel.y < 0) {
                    camera->adjustZoom(camera,-0.1f); // Zoom out
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_SetCursor(mouseDownCursor);
                }
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_SetCursor(customCursor);
                }
            }
            if (enableImGui) {
                ImGui_ImplSDL2_ProcessEvent(&e);
            }
        }
    }

    void update() {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;

        Uint32 elapsedTime = (SDL_GetTicks() - startTime) / 1000;
        if (elapsedTime % 50 < 15) {
            currentPhase = "Day";
        } else if (elapsedTime % 50 < 25) {
            currentPhase = "Dusk";
        } else if (elapsedTime % 50 < 35) {
            currentPhase = "Night";
        } else if (elapsedTime % 50 < 45) {
            currentPhase = "Dawn";
        }
        camera->update(camera,player->getX(), player->getY(), player->getWidth(), player->getHeight(), deltaTime);
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);

        float zoomLevel = camera->getZoomLevel(camera);
        int playerX = player->getX();
        int playerY = player->getY();
        int playerWidth = player->getWidth();
        int playerHeight = player->getHeight();

        // Calculate the zoomed camera position
        int zoomedCameraX = (playerX + playerWidth / 2) - (width / 2) / zoomLevel;
        int zoomedCameraY = (playerY + playerHeight / 2) - (height / 2) / zoomLevel;

        // Apply the zoom level to the rendering
        SDL_RenderSetScale(renderer, zoomLevel, zoomLevel);
        SDL_RenderSetViewport(renderer, nullptr);

        map->draw(renderer, zoomedCameraX, zoomedCameraY, width, height, zoomLevel);
        map->draw_props(renderer, zoomedCameraX, zoomedCameraY, width, height, zoomLevel);
        player->draw(zoomedCameraX, zoomedCameraY, width, height, deltaTime, zoomLevel);
        player2->draw(zoomedCameraX, zoomedCameraY, width, height, deltaTime, zoomLevel); // Draw the second player

        // Reset the scale to 1.0 for ImGui rendering
        SDL_RenderSetScale(renderer, 1.0f, 1.0f);

        if (enableImGui) {
            // Start the ImGui frame
            //ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            // Create an ImGui window
            ImGui::Begin("Main Engine Controller");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("CPU Usage: %.1f%%", getCPUUsage());
            ImGui::Text("GPU Usage: %.1f%%", getGPUUsage());
            ImGui::Text("Current Phase: %s", currentPhase.c_str());

            // Get the current window size
            int windowWidth, windowHeight;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            ImGui::Text("Window Size: %dx%d", windowWidth, windowHeight);

            // Get the player position
            ImGui::Text("Player Position: (%d, %d)", playerX, playerY);

            // Get the mouse position and direction
            int mouseX, mouseY;
            Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
            std::string mouseDirection = getMouseDirection(mouseX, mouseY, playerX, playerY);
            ImGui::Text("Mouse Direction: %s", mouseDirection.c_str());

            // Get the current game time
            Uint32 elapsedTime = SDL_GetTicks() - startTime;
            int days = elapsedTime / (1000 * 60 * 60 * 24);
            int hours = (elapsedTime / (1000 * 60 * 60)) % 24;
            int minutes = (elapsedTime / (1000 * 60)) % 60;
            int seconds = (elapsedTime / 1000) % 60;
            ImGui::Text("Game Time: %02d:%02d:%02d:%02d", days, hours, minutes, seconds);
            ImGui::End();

            // Display the build hash at the left bottom corner
            ImGui::SetNextWindowPos(ImVec2(10, height - 30));
            ImGui::SetNextWindowSize(ImVec2(600, 50));
            ImGui::Begin("Build Hash", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            ImGui::Text("Build Hash: %s", buildHash.c_str());
            ImGui::End();
            // Rendering
            ImGui::Render();
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        }

        SDL_RenderPresent(renderer);
    }

    std::string getMouseDirection(int mouseX, int mouseY, int playerX, int playerY) {
        int dx = mouseX - playerX;
        int dy = mouseY - playerY;

        if (dx > 0 && dy > 0) {
            return "Down-Right";
        } else if (dx > 0 && dy < 0) {
            return "Up-Right";
        } else if (dx < 0 && dy > 0) {
            return "Down-Left";
        } else if (dx < 0 && dy < 0) {
            return "Up-Left";
        } else if (dx > 0) {
            return "Right";
        } else if (dx < 0) {
            return "Left";
        } else if (dy > 0) {
            return "Down";
        } else if (dy < 0) {
            return "Up";
        } else {
            return "Center";
        }
    }

    double getCPUUsage() {
        // Placeholder function to get CPU usage
        // Implement actual CPU usage retrieval logic here
        return 0.0;
    }

    double getGPUUsage() {
        // Placeholder function to get GPU usage
        // Implement actual GPU usage retrieval logic here
        return 0.0;
    }

    void updateTerminalOutput() {
        std::ifstream terminalFile("terminal_output.txt");
        if (terminalFile.is_open()) {
            std::stringstream buffer;
            buffer << terminalFile.rdbuf();
            terminalOutput = buffer.str();
            terminalFile.close();
        }
    }

    void executePythonCommand(const char* command) {
        return;
    }

    SDL_Cursor* createCustomCursor(const char* imagePath, int hotspotX, int hotspotY) {
        SDL_Surface* surface = IMG_Load(imagePath);
        if (!surface) {
            SDL_Log("Could not load cursor image: %s", IMG_GetError());
            return nullptr;
        }

        // Check if the hotspot coordinates are within the bounds of the image
        if (hotspotX < 0 || hotspotX >= surface->w || hotspotY < 0 || hotspotY >= surface->h) {
            SDL_Log("Hotspot coordinates are out of bounds.");
            SDL_FreeSurface(surface);
            return nullptr;
        }

        SDL_Cursor* cursor = SDL_CreateColorCursor(surface, hotspotX, hotspotY);
        SDL_FreeSurface(surface); // Free the surface after creating the cursor

        if (!cursor) {
            SDL_Log("Could not create cursor: %s", SDL_GetError());
        }

        return cursor;
    }

    int width;
    int height;
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    Map* map;
    Player* player;
    Player* player2; // Add the second player
    Camera* camera;
    Uint32 startTime;
    Uint32 lastFrameTime;  // Add this member variable
    float deltaTime;  // Add this member variable
    std::string currentPhase;
    std::string buildHash;  // Store the build hash
    std::string terminalOutput;  // Store the terminal output
    SDL_Cursor* customCursor;
    SDL_Cursor* mouseDownCursor;  // Add this member variable
    bool enableImGui;  // Add this member variable
};
