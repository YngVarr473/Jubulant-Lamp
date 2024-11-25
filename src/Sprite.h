#ifndef SPRITE_H
#define SPRITE_H


SDL_Texture* loadImage(SDL_Renderer* renderer, const char* filePath, int& outWidth, int& outHeight) {
    SDL_Surface* loadedSurface = IMG_Load(filePath);
    if (loadedSurface == NULL) {
        printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (texture == NULL) {
        printf("Unable to create texture! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    outWidth = loadedSurface->w;
    outHeight = loadedSurface->h;
    printf("Loaded image: %s with dimensions %d x %d\n", filePath, outWidth, outHeight);
    return texture;
}

SDL_Surface* scaleSurface(SDL_Surface* surface, int width, int height) {
    SDL_Surface* scaledSurface = SDL_CreateRGBSurface(0, width, height, 32,
                                                     0x00FF0000,
                                                     0x0000FF00,
                                                     0x000000FF,
                                                     0xFF000000);
    if (!scaledSurface) {
        printf("Could not create scaled surface: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SoftStretch(surface, nullptr, scaledSurface, nullptr);
    return scaledSurface;
}

#endif