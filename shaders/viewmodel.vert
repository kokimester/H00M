#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 FragPos0;
flat out ivec4 BoneIDs0;
out vec4 Weights0;

const int MAX_BONES = 200;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 gBones[200];

//SHADOW
/*
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;
uniform mat4 lightSpaceMatrix;
*/
//SHADOW

void main()
{
    mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
    BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
    BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
    BoneTransform     += gBones[BoneIDs[3]] * Weights[3];
    vec4 PosL = BoneTransform * vec4(pos, 1.0);
    mat4 gWVP = projection * model;
    //mat4 gWVP = projection * view * model;
    gl_Position = gWVP * PosL;
    TexCoord0 = tex;
    Normal0 = vec3(view * vec4(norm,1.0));
    FragPos0 = pos;
    BoneIDs0 = BoneIDs;
    Weights0 = Weights;
    /*
    vs_out.FragPos = vec3(BoneTransform * vec4(pos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(BoneTransform))) * norm;
    vs_out.TexCoords = tex;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    */
}