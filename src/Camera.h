
#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Camera Camera;

struct Camera {
    uint32_t viewportWidth, viewportHeight;
    uint32_t mapWidth, mapHeight;
    int x, y, targetX, targetY;
    float easingDuration;
    float zoomLevel;
    float targetZoomLevel;
    void (*update)(Camera*, int playerX, int playerY, int playerWidth, int playerHeight, float deltaTime);
    void (*setViewportSize)(Camera*, int width, int height);
    void (*adjustZoom)(Camera*, float delta);
    float (*getZoomLevel)(Camera* cam);
};

static float getZoomLevel(Camera* cam) {
    return cam->zoomLevel;
}
static float easeInOutQuad(float start, float end, float duration, float elapsed) {
    float t = elapsed / duration;
    t = t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    return start + t * (end - start);
}

static void update(Camera* cam, int playerX, int playerY, int playerWidth, int playerHeight, float deltaTime) {
    cam->targetX = playerX + playerWidth / 2 - cam->viewportWidth / 2;
    cam->targetY = playerY + playerHeight / 2 - cam->viewportHeight / 2;

    if (cam->targetX < 0) cam->targetX = 0;
    if (cam->targetY < 0) cam->targetY = 0;
    if (cam->targetX > cam->mapWidth - cam->viewportWidth) cam->targetX = cam->mapWidth - cam->viewportWidth;
    if (cam->targetY > cam->mapHeight - cam->viewportHeight) cam->targetY = cam->mapHeight - cam->viewportHeight;

    // Interpolate the camera position using an easing function
    cam->x = easeInOutQuad(cam->x, cam->targetX, cam->easingDuration, deltaTime);
    cam->y = easeInOutQuad(cam->y, cam->targetY, cam->easingDuration, deltaTime);

    // Interpolate the zoom level using an easing function
    cam->zoomLevel = easeInOutQuad(cam->zoomLevel, cam->targetZoomLevel, cam->easingDuration, deltaTime);
}

static void adjustZoom(Camera* cam, float delta) {
    cam->targetZoomLevel += delta;
    if (cam->targetZoomLevel < 0.5f) cam->targetZoomLevel = 0.5f;
    if (cam->targetZoomLevel > 2.0f) cam->targetZoomLevel = 2.0f;
}

static void setViewportSize(Camera* cam, int width, int height) {
    cam->viewportWidth = width;
    cam->viewportHeight = height;
}

Camera* createCamera(int viewportWidth, int viewportHeight, int mapWidth, int mapHeight) {
    Camera* cam = (Camera*)malloc(sizeof(Camera));
    if (cam == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    cam->viewportWidth = viewportWidth;
    cam->viewportHeight = viewportHeight;
    cam->mapWidth = mapWidth;
    cam->mapHeight = mapHeight;
    cam->x = 0;
    cam->y = 0;
    cam->targetX = 0;
    cam->targetY = 0;
    cam->easingDuration = 0.1f;
    cam->zoomLevel = 1.0f;
    cam->targetZoomLevel = 1.0f;
    cam->update = update;
    cam->adjustZoom = adjustZoom;
    cam->setViewportSize = setViewportSize;
    cam->getZoomLevel = getZoomLevel;
    return cam;
}


void destroyCamera(Camera* cam) {
    free(cam);
}

#endif // CAMERA_H