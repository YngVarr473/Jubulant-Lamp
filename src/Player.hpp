#ifndef PLAYER_HPP
#define PLAYER_HPP

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
            SDL_Texture* texture = loadImage(renderer, filePath.c_str(), width, height);
            textures.push_back(texture);
        }
        for (const auto& filePath : downTextures) {
            SDL_Texture* texture = loadImage(renderer, filePath.c_str(), width, height);
            textures.push_back(texture);
        }
        for (const auto& filePath : leftTextures) {
            SDL_Texture* texture = loadImage(renderer, filePath.c_str(), width, height);
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


#endif