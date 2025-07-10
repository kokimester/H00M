#version 330

layout (location=0) in vec3 pos;
layout (location=1) in vec2 tex;
layout (location=2) in vec3 norm;

out vec4 vCol;
out vec2 texCoord0;
out vec3 Normal;
out vec3 FragPos;

//SHADOW
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;
uniform mat4 lightSpaceMatrix;
//SHADOW

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;



void main()
{

	gl_Position = projection * view * model * vec4(pos,1.0);

	vCol=vec4(clamp(pos, 0.0f, 1.0f), 1.0f);
	texCoord0 = tex;

	//SHADOW
	vs_out.FragPos = vec3(model * vec4(pos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * norm;
    vs_out.TexCoords = tex;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
	//SHADOW

    //problem is we are smooth shading a hard edged shape
	Normal = mat3(transpose(inverse(model))) * norm;
    // uncomment for normalizing
    // Normal = normalize(mat3(transpose(inverse(mat3(model)))) * norm);

	FragPos = (model * vec4 (pos, 1.0)).xyz;
}
