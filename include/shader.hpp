#define SHADER_HPP

// Vertex shader for the outline
const char* modelOutlineVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader for the outline
const char* modelOutlineFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 0.2, 0.0, 1.0); // Dark green color for the outline
}
)";

// Vertex shader for the model
const char* modelVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

// Fragment shader for the model with textures
const char* modelFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D bodyTexture;
uniform sampler2D eyesTexture;
uniform sampler2D furTexture;

void main() {
    vec4 bodyColor = texture(bodyTexture, TexCoord);
    vec4 eyesColor = texture(eyesTexture, TexCoord);
    vec4 furColor = texture(furTexture, TexCoord);

    // Combine the textures as needed
    FragColor = mix(mix(bodyColor, eyesColor, 0.5), furColor, 0.5);
}
)";


// Вершинный шейдер
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

// Фрагментный шейдер для основной текстуры
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform float pixelSize; // This will control the size of the pixels
uniform int cubeType;
uniform float timeOfDay; // New uniform for time of day

void main()
{
    float Pixels = 512.0;
    // Calculate the pixelated texture coordinates
    vec2 pixelatedTexCoord = floor(TexCoord / pixelSize) * pixelSize;

    // Sample the texture at the pixelated coordinates
    vec2 uv = pixelatedTexCoord;
    vec2 grid = floor(uv * vec2(10.0));

    int gridX = int(grid.x);
    int gridY = int(grid.y);

    // Interpolate colors based on timeOfDay
    vec4 dayColor1 = vec4(0.827, 0.988, 0.498, 1.0); // #d3fc7e
    vec4 dayColor2 = vec4(0.600, 0.902, 0.373, 1.0); // #99e65f
    vec4 nightColor1 = vec4(0.235, 0.235, 0.235, 1.0); // #3d3d3d
    vec4 nightColor2 = vec4(0.153, 0.153, 0.153, 1.0); // #272727

    vec4 color1 = mix(nightColor1, dayColor1, timeOfDay);
    vec4 color2 = mix(nightColor2, dayColor2, timeOfDay);

    if (cubeType == 0) {
        if ((gridX + gridY) % 2 == 0)
        {
            FragColor = color1;
        }
        else
        {
            FragColor = color2;
        }
    } else if (cubeType == 1) {
        if ((gridX + gridY) % 2 == 0)
        {
            FragColor = nightColor1;
        }
        else
        {
            FragColor = nightColor2;
        }
    }
}
)";

const char* geometryShaderSource = R"(
#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = projection * view * model * vec4(gl_in[i].gl_Position.xyz, 1.0);
        TexCoord = gl_in[i].TexCoord;
        EmitVertex();
    }
    EndPrimitive();
}
)";

// Фрагментный шейдер для обводки
const char* outlineFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 0.2, 0.0, 1.0); // Dark green color for the outline
}
)";

// Функция для компиляции шейдера
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}
// Function to create the model shader program
GLuint createModelShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, modelVertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, modelFragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createModelOutlineShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, modelOutlineVertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, modelOutlineFragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Функция для создания шейдерной программы
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath) {
        ID = createShaderProgram(vertexPath, fragmentPath);
    }

    void use() {
        glUseProgram(ID);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
};

