#ifndef COMMON_H
#define COMMON_H

SDL_Cursor* createCustomCursor(const char* imagePath, int hotspotX, int hotspotY, int scale) {
    SDL_Surface* surface = IMG_Load(imagePath);
    if (!surface) {
        printf("Could not load cursor image: %s", IMG_GetError());
        return NULL;
    }

    // Scale the surface
    int scaledWidth = surface->w * scale;
    int scaledHeight = surface->h * scale;
    SDL_Surface* scaledSurface = scaleSurface(surface, scaledWidth, scaledHeight);
    SDL_FreeSurface(surface); // Free the original surface

    if (!scaledSurface) {
        return NULL;
    }

    // Check if the hotspot coordinates are within the bounds of the image
    if (hotspotX < 0 || hotspotX >= scaledSurface->w || hotspotY < 0 || hotspotY >= scaledSurface->h) {
        printf("Hotspot coordinates are out of bounds.");
        SDL_FreeSurface(scaledSurface);
        return NULL;
    }

    SDL_Cursor* cursor = SDL_CreateColorCursor(scaledSurface, hotspotX * scale, hotspotY * scale);
    SDL_FreeSurface(scaledSurface); // Free the scaled surface after creating the cursor

    if (!cursor) {
        printf("Coult not create cursor: %s", SDL_GetError());
    }

    return cursor;
}

#endif