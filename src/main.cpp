#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string loadShaderSource(const std::string& filepath) {
    std::ifstream shaderFile(filepath);
    std::stringstream shaderStream;

    if (shaderFile.is_open()) {
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
    } else {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
    }

    return shaderStream.str();
}

unsigned int compileShader(const std::string& filepath, GLenum shaderType) {
    std::string shaderCode = loadShaderSource(filepath);
    const char* shaderSource = shaderCode.c_str();
    
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error in " << filepath << ":\n" << infoLog << std::endl;
    }
    return shader;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Perspective vs Orthographic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // Shader compilation setup...
    unsigned int vertexShaderOrthographic = compileShader("src\\vs_ortho.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("src\\fs.glsl", GL_FRAGMENT_SHADER);
    unsigned int vertexShaderPerspective = compileShader("src\\vs_perspective.glsl", GL_VERTEX_SHADER);

    unsigned int shaderProgramOrthographic = glCreateProgram();
    glAttachShader(shaderProgramOrthographic, vertexShaderOrthographic);
    glAttachShader(shaderProgramOrthographic, fragmentShader);
    glLinkProgram(shaderProgramOrthographic);

    unsigned int shaderProgramPerspective = glCreateProgram();
    glAttachShader(shaderProgramPerspective, vertexShaderPerspective);
    glAttachShader(shaderProgramPerspective, fragmentShader);
    glLinkProgram(shaderProgramPerspective);

    glDeleteShader(vertexShaderOrthographic);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShaderPerspective);

    // Vertex data for a textured quad
    float vertices[] = {
        // positions         // texture coords
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    // OpenGL buffers setup
    unsigned int VAO[2], VBO, EBO;
    glGenVertexArrays(2, VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    for (int i = 0; i < 2; i++) {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // Load texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("src\\texture.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    bool running = true;
    float angle = 0.0f;
    glEnable(GL_DEPTH_TEST);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Orthographic projection setup
        glm::mat4 projOrthographic = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f); // Orthographic projection matrix
        glm::mat4 modelOrthographic = glm::mat4(1.0f);
        modelOrthographic = glm::rotate(modelOrthographic, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around the Z-axis
        modelOrthographic = glm::translate(modelOrthographic, glm::vec3(0.0f, 0.0f, 0.0f)); // Move to the center side and set z to -1

        glUseProgram(shaderProgramOrthographic);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramOrthographic, "model"), 1, GL_FALSE, glm::value_ptr(modelOrthographic));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramOrthographic, "projection"), 1, GL_FALSE, glm::value_ptr(projOrthographic));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffer
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw orthographic quad


        // **Perspective Projection Object Rendering**
        glUseProgram(shaderProgramPerspective);
        glm::mat4 projPerspective = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 viewPerspective = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.5f));

        // Time-based rotation for perspective quad
        float rotationAngle = SDL_GetTicks() / 1000.0f * glm::radians(50.0f);
        glm::mat4 modelPerspective = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.1f)); // Move left and front
        modelPerspective = glm::rotate(modelPerspective, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis

        glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "model"), 1, GL_FALSE, glm::value_ptr(modelPerspective));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "view"), 1, GL_FALSE, glm::value_ptr(viewPerspective));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "projection"), 1, GL_FALSE, glm::value_ptr(projPerspective));
        glUniform1i(glGetUniformLocation(shaderProgramPerspective, "texture1"), 0);

        glBindVertexArray(VAO[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    // Cleanup resources
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgramOrthographic);
    glDeleteProgram(shaderProgramPerspective);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
