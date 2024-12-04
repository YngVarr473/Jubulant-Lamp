#define SHADER_H_
#include "../include/includes.h"

GLuint compileShader(GLenum type, const char* source);
GLuint create_shader_program(char* vertexSource, char* fragmentSource);

typedef struct {
    GLuint ID;
    void (*use)(struct Shader*);
    void (*set_mat4)(struct Shader*, char* name, mat4 *mat);
    void (*set_float)(struct Shader*, char* name, float value);
    void (*set_int)(struct Shader*, char* name, int value);
} Shader;


void use(Shader* shader);
void set_mat4(Shader* shader, const char* name, mat4 *mat);
void set_float(Shader* shader, const char* name, float value);
void set_int(Shader* shader, const char* name, int value);

Shader* initShader(const char* vertexPath, const char* fragmentPath);

void destroyShader(Shader* shader);
