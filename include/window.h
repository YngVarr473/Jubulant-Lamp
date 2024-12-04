#ifndef WINDOW_H_
#define WINDOW_H_

#include "includes.h"

// --------------- Callbacks.
void framebuffer_size_callback(GLFWwindow* window, uint32_t width, uint32_t height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// --------------- Application Window.
typedef struct ApplicationWindow
{
    GLFWwindow* window;
    void (*event_handler)(struct ApplicationWindow*);
    void (*render)(struct ApplicationWindow*);
};

void event_handler(ApplicationWindow* aw);
void render(ApplicationWindow* aw);
void destroyApplicationWindow(ApplicationWindow* aw);

ApplicationWindow* initApplicationWindow();



#endif // WINDOW_H_
