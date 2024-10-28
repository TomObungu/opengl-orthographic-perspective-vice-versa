#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Time uniform for animating texture coordinates

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);
    
    // Animate the texture coordinates by offsetting with time
    TexCoord = 2 * texCoord.xy + vec2(time * 0.1, time * 0.1); // Adjust speed by changing the multiplier
}
