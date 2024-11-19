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
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <Python.h>  // Include the Python header

const int GRASS_TILE_SIZE = 64;
const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;

SDL_Texture* loadImage(SDL_Renderer* renderer, const std::string& filePath, int& outWidth, int& outHeight) {
    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (texture == nullptr) {
        std::cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    outWidth = loadedSurface->w;
    outHeight = loadedSurface->h;
    std::cout << "Loaded image: " << filePath << " with dimensions " << outWidth << "x" << outHeight << std::endl;
    return texture;
}

class Camera {
public:
    Camera(int viewportWidth, int viewportHeight, int mapWidth, int mapHeight)
        : viewportWidth(viewportWidth), viewportHeight(viewportHeight), mapWidth(mapWidth), mapHeight(mapHeight), x(0), y(0), targetX(0), targetY(0), easingDuration(0.1f), zoomLevel(1.0f), targetZoomLevel(1.0f) {}

    void update(int playerX, int playerY, int playerWidth, int playerHeight, float deltaTime) {
        // Calculate the target position to center the camera on the player
        targetX = playerX + playerWidth / 2 - viewportWidth / 2;
        targetY = playerY + playerHeight / 2 - viewportHeight / 2;

        // Clamp the target position to the map boundaries
        if (targetX < 0) targetX = 0;
        if (targetY < 0) targetY = 0;
        if (targetX > mapWidth - viewportWidth) targetX = mapWidth - viewportWidth;
        if (targetY > mapHeight - viewportHeight) targetY = mapHeight - viewportHeight;

        // Interpolate the camera position using an easing function
        x = easeInOutQuad(x, targetX, easingDuration, deltaTime);
        y = easeInOutQuad(y, targetY, easingDuration, deltaTime);

        // Interpolate the zoom level using an easing function
        zoomLevel = easeInOutQuad(zoomLevel, targetZoomLevel, easingDuration, deltaTime);
    }

    void setViewportSize(int width, int height) {
        viewportWidth = width;
        viewportHeight = height;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    float getZoomLevel() const { return zoomLevel; }

    void adjustZoom(float delta) {
        targetZoomLevel += delta;
        if (targetZoomLevel < 0.5f) targetZoomLevel = 0.5f;
        if (targetZoomLevel > 2.0f) targetZoomLevel = 2.0f;
    }

private:
    int viewportWidth;
    int viewportHeight;
    int mapWidth;
    int mapHeight;
    int x, y;
    int targetX, targetY;
    float easingDuration;
    float zoomLevel;
    float targetZoomLevel;

    float easeInOutQuad(float start, float end, float duration, float elapsed) {
        float t = elapsed / duration;
        t = t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
        return start + t * (end - start);
    }
};

class Player {
public:
    Player(SDL_Renderer* renderer, const std::string& filePath)
        : renderer(renderer), currentFrame(0), frameDelay(0.1f), lastFrameTime(0), direction("down") {
        loadTextures();

        // Calculate the center of the map
        int centerX = (MAP_WIDTH * GRASS_TILE_SIZE) / 2 - width / 2;
        int centerY = (MAP_HEIGHT * GRASS_TILE_SIZE) / 2 - height / 2;

        // Set the player's initial position to the center of the map
        x = centerX;
        y = centerY;

        isMoving = false;
    }

    ~Player() {
        for (auto& texture : textures) {
            SDL_DestroyTexture(texture);
        }
        SDL_DestroyTexture(stayTexture);
    }

    void move(int dx, int dy) {
        x += dx;
        y += dy;

        // Update direction based on movement
        if (dx > 0) direction = "right";
        if (dx < 0) direction = "left";
        if (dy > 0) direction = "down";
        if (dy < 0) direction = "up";
        isMoving = true;
    }

    void draw(int cameraX, int cameraY, int viewportWidth, int viewportHeight, float deltaTime, float zoomLevel) {
        if (isPlayerInView(cameraX, cameraY, viewportWidth, viewportHeight, zoomLevel)) {
            // Update the current frame based on the frame delay
            frameDelay -= deltaTime;
            if (frameDelay <= 0) {
                currentFrame = (currentFrame + 1) % 6;
                frameDelay = 0.1f;
            }

            SDL_Rect destRect = { x - cameraX, y - cameraY, width * 2, height * 2 };
            SDL_RendererFlip flip = SDL_FLIP_NONE;

            // Select the appropriate texture based on the direction and current frame
            SDL_Texture* texture = nullptr;
            if (isMoving) {
                if (direction == "up") {
                    texture = textures[currentFrame];
                } else if (direction == "down") {
                    texture = textures[currentFrame + 6];
                } else if (direction == "left") {
                    texture = textures[currentFrame + 12];
                } else if (direction == "right") {
                    texture = textures[currentFrame + 12];
                    flip = SDL_FLIP_HORIZONTAL;
                }
            } else {
                texture = stayTexture;
            }

            SDL_RenderCopyEx(renderer, texture, nullptr, &destRect, 0, nullptr, flip);
        }
    }


    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void setMoving(bool moving) {
        isMoving = moving;
    }
    bool isPlayerInView(int cameraX, int cameraY, int viewportWidth, int viewportHeight, float zoomLevel) {
    int playerX = x;
    int playerY = y;
    int playerWidth = width * 2;
    int playerHeight = height * 2;

    int viewX = cameraX;
    int viewY = cameraY;
    int viewWidth = viewportWidth / zoomLevel;
    int viewHeight = viewportHeight / zoomLevel;

    return !(playerX + playerWidth < viewX || playerX > viewX + viewWidth ||
             playerY + playerHeight < viewY || playerY > viewY + viewHeight);
}

private:
    void loadTextures() {
        std::vector<std::string> upTextures = {
            "../Assets/Character/u1.png", "../Assets/Character/u2.png", "../Assets/Character/u3.png",
            "../Assets/Character/u4.png", "../Assets/Character/u5.png", "../Assets/Character/u6.png"
        };
        std::vector<std::string> downTextures = {
            "../Assets/Character/d1.png", "../Assets/Character/d2.png", "../Assets/Character/d3.png",
            "../Assets/Character/d4.png", "../Assets/Character/d5.png", "../Assets/Character/d6.png"
        };
        std::vector<std::string> leftTextures = {
            "../Assets/Character/w1.png", "../Assets/Character/w2.png", "../Assets/Character/w3.png",
            "../Assets/Character/w4.png", "../Assets/Character/w5.png", "../Assets/Character/w6.png"
        };

        for (const auto& filePath : upTextures) {
            SDL_Texture* texture = loadImage(renderer, filePath, width, height);
            textures.push_back(texture);
        }
        for (const auto& filePath : downTextures) {
            SDL_Texture* texture = loadImage(renderer, filePath, width, height);
            textures.push_back(texture);
        }
        for (const auto& filePath : leftTextures) {
            SDL_Texture* texture = loadImage(renderer, filePath, width, height);
            textures.push_back(texture);
        }
        stayTexture = loadImage(renderer, "../Assets/Character/s1.png", width, height);
    }

    SDL_Renderer* renderer;
    bool isMoving;
    int x, y;
    int width, height;
    SDL_Texture* stayTexture;
    std::vector<SDL_Texture*> textures;
    int currentFrame;
    float frameDelay;
    Uint32 lastFrameTime;
    std::string direction;
};



class Map {
public:
    Map(SDL_Renderer* renderer, int width, int height, int tileSize)
        : width(width), height(height), tileSize(tileSize), renderer(renderer) {
        grassTexture = loadImage(renderer, "../Assets/grass.png", grassWidth, grassHeight);
        waterTexture = loadImage(renderer, "../Assets/water.png", waterWidth, waterHeight);
        forestTexture = loadImage(renderer, "../Assets/forest.png", grassWidth, grassHeight);

        rockBigTexture = loadImage(renderer, "../Assets/rock_big.png", waterWidth, waterHeight);
        rockMediumTexture = loadImage(renderer, "../Assets/rock_medium.png", waterWidth, waterHeight);
        flower1Texture = loadImage(renderer, "../Assets/romashka_big.png", waterWidth, waterHeight);
        flower2texture = loadImage(renderer, "../Assets/flower2_big.png", waterWidth, waterHeight);
        bush1Texture= loadImage(renderer, "../Assets/bush_big.png", waterWidth, waterHeight);
        mapData = generateMap();
        propsData = generateProps(mapData);
    }

    ~Map() {
        SDL_DestroyTexture(grassTexture);
        SDL_DestroyTexture(waterTexture);
        SDL_DestroyTexture(rockBigTexture);
        SDL_DestroyTexture(forestTexture);
        SDL_DestroyTexture(rockBigTexture);
        SDL_DestroyTexture(rockMediumTexture);
        SDL_DestroyTexture(flower1Texture);
        SDL_DestroyTexture(flower2texture);
        SDL_DestroyTexture(bush1Texture);
    }

    void draw(SDL_Renderer* renderer, int cameraX, int cameraY, int viewportWidth, int viewportHeight, float zoomLevel) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (isTileInView(x, y, cameraX, cameraY, viewportWidth, viewportHeight, zoomLevel)) {
                    SDL_Rect destRect;
                    destRect.x = x * tileSize - cameraX;
                    destRect.y = y * tileSize - cameraY;
                    destRect.w = tileSize;
                    destRect.h = tileSize;

                    if (mapData[y][x] == 0) {
                        SDL_RenderCopy(renderer, waterTexture, nullptr, &destRect);
                    } else if (mapData[y][x] == 2) {
                        SDL_RenderCopy(renderer, grassTexture, nullptr, &destRect);
                    } else if (mapData[y][x] == 1) {
                        SDL_RenderCopy(renderer, grassTexture, nullptr, &destRect);
                    }
                }
            }
        }
    }

    void draw_props(SDL_Renderer* renderer, int cameraX, int cameraY, int viewportWidth, int viewportHeight, float zoomLevel) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (isTileInView(x, y, cameraX, cameraY, viewportWidth, viewportHeight, zoomLevel)) {
                    SDL_Rect destRect;
                    destRect.x = x * tileSize - cameraX;
                    destRect.y = y * tileSize - cameraY;
                    destRect.w = tileSize;
                    destRect.h = tileSize;

                    if (propsData[y][x] == 3) {
                        SDL_RenderCopy(renderer, forestTexture, nullptr, &destRect);
                    } else if (propsData[y][x] == 4) {
                        SDL_RenderCopy(renderer, rockBigTexture, nullptr, &destRect);
                    } else if (propsData[y][x] == 5) {
                        SDL_RenderCopy(renderer, rockMediumTexture, nullptr, &destRect);
                    } else if (propsData[y][x] == 6) {
                        SDL_RenderCopy(renderer, flower1Texture, nullptr, &destRect);
                    } else if (propsData[y][x] == 7) {
                        SDL_RenderCopy(renderer, flower2texture, nullptr, &destRect);
                    } else if (propsData[y][x] == 8) {
                        SDL_RenderCopy(renderer, bush1Texture, nullptr, &destRect);
                    }
                }
            }
        }
    }

    bool isTileInView(int x, int y, int cameraX, int cameraY, int viewportWidth, int viewportHeight, float zoomLevel) {
        int tileX = x * tileSize;
        int tileY = y * tileSize;
        int tileWidth = tileSize;
        int tileHeight = tileSize;

        int viewX = cameraX;
        int viewY = cameraY;
        int viewWidth = viewportWidth / zoomLevel;
        int viewHeight = viewportHeight / zoomLevel;

        return !(tileX + tileWidth < viewX || tileX > viewX + viewWidth ||
                tileY + tileHeight < viewY || tileY > viewY + viewHeight);
    }


private:
    std::vector<std::vector<int>> generateMap() {
        std::vector<std::vector<int>> map_data(height, std::vector<int>(width, 0));

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                    map_data[i][j] = 0;  // Water tile
                }
            }
        }

        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                int tile = std::rand() % 11;  // Generate a random number between 0 and 10
                if (tile < 2) {  // 20% chance for water
                    map_data[i][j] = 0;
                } else if (tile < 5) {  // 30% chance for sand
                    map_data[i][j] = 1;
                } else {  // 50% chance for grass
                    map_data[i][j] = 2;
                }
            }
        }

        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                if (map_data[i][j] == 0) {
                    int neighbors[4][2] = {
                        {i-1, j}, {i+1, j}, {i, j-1}, {i, j+1}
                    };
                    bool has_water_neighbor = false;
                    for (int k = 0; k < 4; k++) {
                        int ni = neighbors[k][0];
                        int nj = neighbors[k][1];
                        if (ni >= 0 && ni < height && nj >= 0 && nj < width && map_data[ni][nj] == 0) {
                            has_water_neighbor = true;
                            break;
                        }
                    }
                    if (!has_water_neighbor) {
                        map_data[i][j] = 2;  // Replace isolated water with grass
                    }
                }
            }
        }

        std::cout << "Map data generated successfully." << std::endl;
        return map_data;
    }

    std::vector<std::vector<int>> generateProps(const std::vector<std::vector<int>>& mapData) {
        std::vector<std::vector<int>> props_data(height, std::vector<int>(width, 0));

        // Place props on grass tiles
        for (int i = 1; i < height - 1; i++) {
            for (int j = 1; j < width - 1; j++) {
                if (mapData[i][j] == 2) {  // Grass tile
                    int prop = std::rand() % 100;  // Generate a random number between 0 and 99
                    if (prop < 5) {  // 5% chance for forest
                        props_data[i][j] = 3;
                    } else if (prop < 10) {  // 5% chance for rock big
                        props_data[i][j] = 4;
                    } else if (prop < 15) {  // 5% chance for rock medium
                        props_data[i][j] = 5;
                    } else if (prop < 20) {  // 5% chance for flower 1
                        props_data[i][j] = 6;
                    } else if (prop < 25) {  // 5% chance for flower 2
                        props_data[i][j] = 7;
                    } else if (prop < 30) {  // 5% chance for bush 1
                        props_data[i][j] = 8;
                    }
                }
            }
        }

        std::cout << "Props data generated successfully." << std::endl;
        return props_data;
    }

    int width;
    int height;
    int tileSize;
    SDL_Renderer* renderer;
    SDL_Texture* grassTexture;
    SDL_Texture* waterTexture;
    SDL_Texture* forestTexture;
    SDL_Texture* rockBigTexture;
    SDL_Texture* rockMediumTexture;
    SDL_Texture* flower1Texture;
    SDL_Texture* flower2texture;
    SDL_Texture* bush1Texture;
    int grassWidth;
    int grassHeight;
    int waterWidth;
    int waterHeight;
    std::vector<std::vector<int>> mapData;
    std::vector<std::vector<int>> propsData;
};

class Game {
public:
    Game() : width(800), height(600), running(true), lastFrameTime(0), deltaTime(0.0f) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        window = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            exit(1);
        }

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        // Initialize Python interpreter
        Py_Initialize();

        map = new Map(renderer, MAP_WIDTH, MAP_HEIGHT, GRASS_TILE_SIZE);
        player = new Player(renderer, "../Assets/Character/s1.png");
        player2 = new Player(renderer, "../Assets/Character/s1.png"); // Create the second player
        camera = new Camera(width, height, MAP_WIDTH * GRASS_TILE_SIZE, MAP_HEIGHT * GRASS_TILE_SIZE);
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

        // Redirect stdout and stderr to the terminal
        std::freopen("terminal_output.txt", "w", stdout);
        std::freopen("terminal_output.txt", "w", stderr);
    }

    ~Game() {
        delete map;
        delete player;
        delete player2; // Clean up the second player
        delete camera;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        // Cleanup ImGui
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        // Finalize Python interpreter
        Py_Finalize();
    }

    void run() {
        camera->update(player->getX(), player->getY(), player->getWidth(), player->getHeight(), 0);
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
                camera->setViewportSize(width, height);
                camera->update(player->getX(), player->getY(), player->getWidth(), player->getHeight(), 0.0f); // Pass 0 for deltaTime initially
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
                    camera->adjustZoom(0.1f); // Zoom in
                } else if (e.wheel.y < 0) {
                    camera->adjustZoom(-0.1f); // Zoom out
                }
            }
            ImGui_ImplSDL2_ProcessEvent(&e);
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
        camera->update(player->getX(), player->getY(), player->getWidth(), player->getHeight(), deltaTime);
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);

        float zoomLevel = camera->getZoomLevel();
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

        // Start the ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Create an ImGui window
        ImGui::Begin("Game Info");
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
        ImGui::SetNextWindowSize(ImVec2(600, 20));
        ImGui::Begin("Build Hash", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("Build Hash: %s", buildHash.c_str());
        ImGui::End();

        // Display the terminal window
        ImGui::SetNextWindowSize(ImVec2(width / 2, height / 2), ImGuiCond_FirstUseEver);
        ImGui::Begin("Terminal");
        ImGui::TextUnformatted(terminalOutput.c_str());

        // Input field for Python commands
        static char pythonCommand[256] = "";
        ImGui::InputText("Python Command", pythonCommand, IM_ARRAYSIZE(pythonCommand));
        ImGui::SameLine();
        if (ImGui::Button("Execute")) {
            executePythonCommand(pythonCommand);
            memset(pythonCommand, 0, sizeof(pythonCommand));  // Clear the input field
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

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
};
