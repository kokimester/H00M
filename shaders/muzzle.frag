#version 330

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 FragPos0;

out vec4 color;

uniform sampler2D diffuseTexture;
uniform float progress;

void main()
{
    //this way alpha is also included
    color = texture(diffuseTexture,TexCoord0);
    color.a *= 1.0-progress;
}
