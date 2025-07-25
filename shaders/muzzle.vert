#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 FragPos0;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	gl_Position = projection * view * model * vec4(pos,1.0);
	gl_Position = projection * model * vec4(pos,1.0);

    TexCoord0 = tex;
	Normal0 = mat3(transpose(inverse(model))) * norm;
	FragPos0 = (model * vec4 (pos, 1.0)).xyz;
}
