#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view; // Added view matrix
uniform mat4 projection;

uniform mat4 transform;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f); // Include view matrix in transformation
    TexCoord = texCoord;
}
