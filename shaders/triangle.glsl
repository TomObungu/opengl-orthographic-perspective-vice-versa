#version 330 core
layout (location = 0) in vec3 AttributePosition;

void main()
{
    gl_Position = vec4 (AttributePosition.x,AttributePosition.y,AttributePosition.z,1.0)
}
