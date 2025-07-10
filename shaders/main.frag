#version 330

in vec4 vCol;
in vec2 texCoord0;
// flat in vec3 Normal;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D shadowMap;
uniform sampler2D diffuseTexture;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

//SHADOW
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;
//SHADOW

struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};

struct Spotlight
{
	PointLight base;
	vec3 direction;
	float edge;
    //used for smooth spotlight cutoff
    float innerCutoff;// cos(inner angle)
    float outerCutoff;// cos(outer angle)
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform Spotlight spotLights[MAX_SPOT_LIGHTS];
uniform DirectionalLight directionalLight;

uniform sampler2D theTexture;
uniform Material material;

uniform vec3 eyePosition;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;

	float diffuseFactor = max(dot(normalize(Normal) , normalize(direction)), 0.0f);
	vec4 diffuseColor = vec4(light.color, 1.0f) * light.diffuseIntensity * diffuseFactor;

	vec4 specularColor = vec4(0,0,0,0);

	if(diffuseFactor > 0.0f)
	{
        //this code makes it so if you look at a wall at an angle
        //you will not see the light reflecting back to your eye
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));

		float specularFactor = dot(fragToEye, reflectedVertex);
		if (specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	return (ambientColor + diffuseColor + specularColor);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = FragPos - pLight.position;
		float distance = length(direction);
		direction = normalize(direction);

		vec4 color = CalcLightByDirection(pLight.base, direction);
		float attenuation = pLight.exponent * distance * distance +
							pLight.linear * distance +
							pLight.constant;
		return (color / attenuation);
}

vec4 CalcSpotLight(Spotlight sLight)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);

    float theta = dot(rayDirection, sLight.direction);
    float epsilon = sLight.innerCutoff - sLight.outerCutoff;
    float intensity = clamp((theta - sLight.outerCutoff) / epsilon, 0.0, 1.0);

    vec4 baseLight = CalcPointLight(sLight.base);
    return baseLight * intensity;

    //dead code after this
	if (slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base);
		return color * (1.0f - (1.0f - slFactor)*(1.0f / (1.0f - sLight.edge)));
	}
	else
	{
		return vec4(0,0,0,0);
	}
}

vec4 CalcSpotLights()
{
	vec4 totalColor = vec4(0,0,0,0);
	 for(int i=0;i<spotLightCount;i++)
	 {
		totalColor += CalcSpotLight(spotLights[i]);
	 }
	return totalColor;
}

vec4 CalcPointLights()
{
	 vec4 totalColor = vec4(0,0,0,0);
	 for(int i=0;i<pointLightCount;i++)
	 {
		totalColor += CalcPointLight(pointLights[i]);
	 }
	return totalColor;
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(Normal, spotLights[0].direction)), 0.005); 
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  

    return shadow;
}
void main()
{
	vec4 finalColor = CalcDirectionalLight();
	finalColor += CalcPointLights();
	finalColor += CalcSpotLights();
    // 10% ambient light in case there is not enough light
    //finalColor += vec4(0.1, 0.1, 0.1, 0.0);
	vec3 textureColor = texture(diffuseTexture, fs_in.TexCoords).rgb;
	//SHADOW
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (1.0 - shadow) * textureColor;    
    //lighting = vec3(shadow);
    color = vec4(lighting, 1.0) * finalColor;

    //uncomment to check normals
    //color = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);

}
