#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_glfw.h"
#include "../include/imgui/imgui_impl_opengl3.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "../include/shader.hpp"

#include "../include/cube.hpp"
#include "../include/plane.hpp"
#include "../include/mesh.hpp"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.1f, float pitch = -28.3f)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

// Global camera object
Camera camera(glm::vec3(0.0f, 9.84f, 14.36f));

// Callback function for framebuffer resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback function for mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    // Check if the left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

// Callback function for mouse scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

GLuint loadTexture(const char* path) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    std::cout << "Loaded texture: " << path << " with ID: " << textureID << std::endl;
    return textureID;
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube with Checkerboard Pattern", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Установка контекста
    glfwMakeContextCurrent(window);

    // Установка callback для изменения размера окна
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Инициализация ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
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

    // Установка viewport
    glViewport(0, 0, 800, 600);

    // Установка цвета фона
    glClearColor(91.0f / 255.0f, 119.0f / 255.0f, 225.0f / 255.0f, 1.0f);

    // Включение проверки глубины
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Включение буфера трафарета
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Создание шейдерной программы для основной текстуры
    Shader shader(vertexShaderSource, fragmentShaderSource);

    // Создание шейдерной программы для обводки
    Shader outlineShader(vertexShaderSource, outlineFragmentShaderSource);
    Shader modelShader(modelVertexShaderSource, modelFragmentShaderSource);
    Shader modelOutlineShader(modelOutlineVertexShaderSource, modelOutlineFragmentShaderSource);
    // Создание кубов
    std::vector<Cube> cubes = {
        Cube(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), 1),
        Cube(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(45.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), 1, true, 12.0f)
    };

    // Создание плоскости
    Plane plane(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f), 0);

    Mesh humanModel = loadModel("../Assets/rigged_human.obj");
    std::cout << "Model loaded!" << std::endl;
    Mesh wolfModel = loadModel("../Assets/Objects/wolf/obj/Wolf_obj.obj");
    std::cout << "Model loaded!" << std::endl;

    // Load textures for the wolf model
    GLuint wolfBodyTexture = loadTexture("../Assets/Objects/wolf/obj/textures/Wolf_Body.jpg");
    GLuint wolfEyesTexture = loadTexture("../Assets/Objects/wolf/obj/textures/Wolf_Eyes_2.jpg");
    GLuint wolfFurTexture = loadTexture("../Assets/Objects/wolf/obj/textures/Wolf_Fur.jpg");

    // Load texture for the plane
    GLuint planeTexture = loadTexture("../Assets/skin_texture.jpg");

    // Time of day variable
    float timeOfDay = 0.5f; // 0.0 for night, 1.0 for day
    float timeSpeed = 0.01f; // Speed of time change

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Обработка ввода
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, 0.05f);

        // Add arrow key controls
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, 0.05f);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, 0.05f);

        // Update time of day
        timeOfDay += timeSpeed * ImGui::GetIO().DeltaTime;
        if (timeOfDay > 1.0f)
            timeOfDay = 0.0f;

        // Рендеринг
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Установка матриц
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);

        /// HUMAN MODEL
        modelShader.use();
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);
        glm::mat4 model = glm::mat4(1.0f); // Identity matrix for the model
        model = glm::translate(model, glm::vec3(-1.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f)); // FIXME: scale factor = ...
        modelShader.setMat4("model", model);
        humanModel.Draw(modelShader);
        // HUMAN MODEL

        /// WOLF MODEL
        // In the rendering loop
        modelShader.use();
        modelShader.setMat4("view", view);
        modelShader.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wolfBodyTexture);
        modelShader.setInt("bodyTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wolfEyesTexture);
        modelShader.setInt("eyesTexture", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, wolfFurTexture);
        modelShader.setInt("furTexture", 2);

        glm::mat4 wmodel = glm::mat4(1.0f); // Identity matrix for the model
        wmodel = glm::translate(wmodel, glm::vec3(-1.5f, -1.0f, 0.0f));
        wmodel = glm::scale(wmodel, glm::vec3(1.0f, 1.0f, 1.0f)); // FIXME: scale factor = ...
        modelShader.setMat4("model", wmodel);
        wolfModel.Draw(modelShader);

        // WOLF MODEL

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        outlineShader.use();
        outlineShader.setMat4("view", view);
        outlineShader.setMat4("projection", projection);

        // Рисование кубов
        for (auto& cube : cubes) {
            // Рисование куба в буфер трафарета
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
            shader.use();
            float pixelSize = 0.01f; // You can adjust this value to change the pixelation effect
            shader.setFloat("pixelSize", pixelSize);
            shader.setFloat("timeOfDay", timeOfDay); // Set the time of day
            cube.draw(shader);
        }

        // Рисование плоскости в буфер трафарета
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        shader.use();
        float pixelSize = 0.001f; // You can adjust this value to change the pixelation effect
        shader.setFloat("pixelSize", pixelSize);
        shader.setFloat("timeOfDay", timeOfDay); // Set the time of day

        // Bind the plane texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        shader.setInt("texture1", 0);

        plane.draw(shader);

        // Рисование обводки только в областях перекрытия
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        outlineShader.use();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(20.0f); // Установка толщины линии для обводки

        for (auto& cube : cubes) {
            cube.updateRotation(ImGui::GetIO().DeltaTime);
            cube.draw(outlineShader);
        }

        plane.draw(outlineShader);

        // Рисование основной текстуры
        glStencilMask(0xFF);
        glEnable(GL_DEPTH_TEST);
        shader.use();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for (auto& cube : cubes) {
            cube.draw(shader);
        }

        plane.draw(shader);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window
        ImGui::Begin("Settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        glm::vec3 cameraPos = camera.Position;
        ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);

        // Time of day slider
        ImGui::SliderFloat("Time of Day", &timeOfDay, 0.0f, 1.0f);

        ImGui::End();

        // Rendering ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Обмен буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Очистка
    for (auto& cube : cubes) {
        glDeleteVertexArrays(1, &cube.VAO);
        glDeleteBuffers(1, &cube.VBO);
    }

    glDeleteVertexArrays(1, &plane.VAO);
    glDeleteBuffers(1, &plane.VBO);

    // Cleanup
    glDeleteVertexArrays(1, &humanModel.VAO);
    glDeleteBuffers(1, &humanModel.VBO);
    glDeleteBuffers(1, &humanModel.EBO);

    // Cleanup
    glDeleteVertexArrays(1, &wolfModel.VAO);
    glDeleteBuffers(1, &wolfModel.VBO);
    glDeleteBuffers(1, &wolfModel.EBO);

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Завершение GLFW
    glfwTerminate();
    return 0;
}
