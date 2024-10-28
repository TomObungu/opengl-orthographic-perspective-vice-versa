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

// Function to load shader file
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
// Function to compile shader GLSL files
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
    int windowWidth, windowHeight;
    SDL_DisplayMode displayMode;
    SDL_Init(SDL_INIT_EVERYTHING);
     if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not get display mode: %s", SDL_GetError());
    }
    else
    {
        windowWidth = displayMode.w;
        windowHeight = displayMode.h;
    }

    // Create the window in fullscreen mode
    SDL_Window* window= SDL_CreateWindow("Perspective vs Orthographic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    // Shader compilation setup...
    unsigned int vertexShaderOrthographic = compileShader("shaders\\vs_ortho.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("shaders\\fs.glsl", GL_FRAGMENT_SHADER);
    unsigned int vertexShaderPerspective = compileShader("shaders\\vs_perspective.glsl", GL_VERTEX_SHADER);

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
    float vertices[2][20] = {

        // Orthographic projection quad
        {-1.0f, -1.0, 0.0f,  0.0f, 0.0f,  // Bottom-left
        1.0f, -1.0, 0.0f,  1.0f, 0.0f,  // Bottom-right
        1.0f,  1.0, 0.0f,  1.0f, 1.0f,  // Top-right
        -1.0f,  1.0, 0.0f,  0.0f, 1.0f},   // Top-left

        // Perspective projection quad
        // Slightly different as I was having issues with aspect ratio and sizing of the texture (varible depening on resolution)
        // positions           // texture coordinates
        {-2.0f, -1.125, 0.0f,  0.0f, 0.0f,  // Bottom-left
        2.0f, -1.125, 0.0f,  1.0f, 0.0f,  // Bottom-right
        2.0f,  1.125, 0.0f,  1.0f, 1.0f,  // Top-right
        -2.0f,  1.125, 0.0f,  0.0f, 1.0f}, // Top-left
    };
    
    // Index buffer to save manual definition and repetition of coordinates within the vertex specification
    // Used to make a rectangle out of triangles
    // Coordinates of the triangle corners are draw in this order :
        // corners : bottom left, bottom right, top right, top right(2nd triangle), top left (2nd triangle), bottom left (2nd triangle)
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    // OpenGL buffers setup
    // Set up Vertex Attribute Objects, Vertex Buffer Objects and Index/Element buffers for the data in arrays
    unsigned int VAO[2], VBO[2], EBO;
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);

    // Use for loop to first bind the vertices for orthgraphic projection then vertices for perspective projection
    for (int i = 0; i < 2; i++) {
        glBindVertexArray(VAO[i]);
        // Bind buffer containing vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        // Set way data is proccesed to STATIC_DRAW as data does not change however it used frequently
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[i]), vertices[i], GL_STATIC_DRAW);
        // Bind buffer contain the indices for the vertex data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Set the first attribute of the vertex attribute to the vertex coordinates
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Set the second attribute of the vertx attribute to the texture coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // Load texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set textures to repeat and wrap around when on the x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     stbi_set_flip_vertically_on_load(true);

    //Load texture using stdbi library
    // Must use GL_RGBA as texture is PNG format
    int width, height, nrChannels;
    unsigned char* data = stbi_load("src\\texture.png", &width, &height, &nrChannels, 0);
    std::cout << width << height;
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    bool running = true;
    float angle = 0.0f;

    // Disabling depth testing means that things get rendered in the order you render them
    glEnable(GL_DEPTH_TEST);
    // Enabling means the perspective will phase through the orthographic due to ovelapping z coordinates

// Render loop
while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
    }

    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Compute time in seconds
    float timeValue = SDL_GetTicks() / 1000.0f;

    // **Orthographic Projection Object Rendering**
    
    // Use orthgraphic projection shader
    glUseProgram(shaderProgramOrthographic);

    // Set up orthographic projection matrix to map directly to window dimensions
    glm::mat4 projOrtho = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), -1.0f, 1.0f);

    glm::mat4 modelOrtho = glm::mat4(1.0f);

    // Step 1: Translate the vertices multiplied by the orthographic projection matrix to the center of the screen
    modelOrtho = glm::translate(modelOrtho, glm::vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f));
    // Step 1.5 : Set the vertices multiplied by the orthographic projection matrix to the size of the screen
    // (I was having a bug in which the object being rendered was minuscule)
    modelOrtho = glm::scale(modelOrtho, glm::vec3(windowWidth, windowHeight, 0.0f));

    // Step 2: Scale the object's vertices that are multiplied by the orthographic projection matrix quad to 50% size
    modelOrtho = glm::scale(modelOrtho, glm::vec3(0.25f, 0.25f, 1.0f));

    // Pass the model matrix to the shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramOrthographic, "model"), 1, GL_FALSE, glm::value_ptr(modelOrtho));

    // Set uniforms for the orthographic vertex shader
    int projLoc = glGetUniformLocation(shaderProgramOrthographic, "projection");
    int modelLoc = glGetUniformLocation(shaderProgramOrthographic, "model");
     
    // Error handling; If the unifroms are valid, update the unifroms
    if (projLoc != -1) {
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projOrtho));
    }
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelOrtho));
    }

    // Update the time uniform within the orthographic vertex shader
    GLint timeLocOrthoggraphic= glGetUniformLocation(shaderProgramOrthographic, "time");
    glUniform1f(timeLocOrthoggraphic, timeValue);

    // Set texture uniform
    glUniform1i(glGetUniformLocation(shaderProgramOrthographic, "texture1"), 0);

    // Draw the 2D orthographic quad
    glBindVertexArray(VAO[0]);
    // Draw the triangles that make up the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);   

    // **Perspective Projection Object Rendering**

    glm::mat4 projPerspective = glm::mat4(1.0f);  // Identity matrix for projection
    glm::mat4 viewPerspective = glm::mat4(1.0f);  // Identity matrix for view

    // Use perspective projection shader
    glUseProgram(shaderProgramPerspective);
    // Set up perspective projection matrix, set up fov, width/hieght, near/far plane
    projPerspective = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    // Set up view matrix and translate the projection matrix in the reverse direction by 2 units
    viewPerspective = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));

    // increment rotation angle by 50 degrees (convert degrees to radians)
    float rotationAngle = timeValue * glm::radians(50.0f);
    // Apply translation to objects by multiplying the projection by model matrix
    // First application - A translation of the object's vertices that are multiplied by the perspective matrix to the center of the screen
    glm::mat4 modelPerspective = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    // Second application - A scale of the object's vertices that are multiplied by the perspective matrixby scale factor 0.5
    modelPerspective = glm::scale(modelPerspective, glm::vec3(0.5f, 0.5f, 1.0f));
    // Third application - Rotate the oobject's vertices that are multiplied by the perspective matrix around the y-axis continually by time
    modelPerspective = glm::rotate(modelPerspective, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Pass time uniform to perspective shader; Update the time uniform 
    GLint timeLocPerspective = glGetUniformLocation(shaderProgramPerspective, "time");
    glUniform1f(timeLocPerspective, timeValue);

    // Set other uniforms for perspective vertex shader shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "model"), 1, GL_FALSE, glm::value_ptr(modelPerspective));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "view"), 1, GL_FALSE, glm::value_ptr(viewPerspective));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramPerspective, "projection"), 1, GL_FALSE, glm::value_ptr(projPerspective));
    // Both the perspective and orthographic shaders are using the same fragment shaders
    glUniform1i(glGetUniformLocation(shaderProgramPerspective, "texture1"), 0);

    // Draw perspective quad; Textures bound will be rendered here via the fragment shader
    glBindVertexArray(VAO[1]);
    // Render the triangles that make up quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Swap buffers
    SDL_GL_SwapWindow(window);
}
    // Cleanup resources
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgramOrthographic);
    glDeleteProgram(shaderProgramPerspective);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
