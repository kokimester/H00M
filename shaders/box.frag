#version 330 core

out vec4 FragColor;
flat in vec3 Color;

void main()
{
    float transparency = 0.5f;
    FragColor = vec4(Color, transparency);
}
