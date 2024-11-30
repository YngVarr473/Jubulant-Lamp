#ifndef MAP_HPP
#define MAP_HPP


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


#endif
