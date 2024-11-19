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
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

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
        : viewportWidth(viewportWidth), viewportHeight(viewportHeight), mapWidth(mapWidth), mapHeight(mapHeight), x(0), y(0) {}

    void update(int playerX, int playerY, int playerWidth, int playerHeight) {
        // Center the camera on the player
        x = playerX + playerWidth / 2 - viewportWidth / 2;
        y = playerY + playerHeight / 2 - viewportHeight / 2;

        // Clamp the camera to the map boundaries
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > mapWidth - viewportWidth) x = mapWidth - viewportWidth;
        if (y > mapHeight - viewportHeight) y = mapHeight - viewportHeight;
    }

    void setViewportSize(int width, int height) {
        viewportWidth = width;
        viewportHeight = height;
    }

    int getX() const { return x; }
    int getY() const { return y; }

private:
    int viewportWidth;
    int viewportHeight;
    int mapWidth;
    int mapHeight;
    int x, y;
};

class Player {
public:
    Player(SDL_Renderer* renderer, const std::string& filePath)
        : renderer(renderer), x(0), y(0) {
        texture = loadImage(renderer, filePath, width, height);
    }

    ~Player() {
        SDL_DestroyTexture(texture);
    }

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void draw(int cameraX, int cameraY) {
        SDL_Rect destRect = { x - cameraX, y - cameraY, width * 2, height *2};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int x, y;
    int width, height;
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

    void draw(SDL_Renderer* renderer, int cameraX, int cameraY) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
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
        std::cout << "Map drawn successfully." << std::endl;
    }

    void draw_props(SDL_Renderer* renderer, int cameraX, int cameraY) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
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
        std::cout << "Props drawn successfully." << std::endl;
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
    Game() : width(800), height(600), running(true) {
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

        map = new Map(renderer, MAP_WIDTH, MAP_HEIGHT, GRASS_TILE_SIZE);
        player = new Player(renderer, "../Assets/Character/s1.png");
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
    }

    ~Game() {
        delete map;
        delete player;
        delete camera;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        // Cleanup ImGui
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void run() {
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
                camera->update(player->getX(), player->getY(), player->getWidth(), player->getHeight());
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        player->move(0, -10);
                        break;
                    case SDLK_DOWN:
                        player->move(0, 10);
                        break;
                    case SDLK_LEFT:
                        player->move(-10, 0);
                        break;
                    case SDLK_RIGHT:
                        player->move(10, 0);
                        break;
                }
            }
            ImGui_ImplSDL2_ProcessEvent(&e);
        }
    }

    void update() {
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
        camera->update(player->getX(), player->getY(), player->getWidth(), player->getHeight());
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);
        map->draw(renderer, camera->getX(), camera->getY());
        map->draw_props(renderer, camera->getX(), camera->getY());
        player->draw(camera->getX(), camera->getY());

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
        ImGui::End();

        // Display the build hash at the left bottom corner
        ImGui::SetNextWindowPos(ImVec2(10, height - 30));
        ImGui::SetNextWindowSize(ImVec2(200, 20));
        ImGui::Begin("Build Hash", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::Text("Build Hash: %s", buildHash.c_str());
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
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

    int width;
    int height;
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    Map* map;
    Player* player;
    Camera* camera;
    Uint32 startTime;
    std::string currentPhase;
    std::string buildHash;  // Store the build hash
};
