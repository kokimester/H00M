#version 330

in vec4 vCol;
in vec2 texCoord0;
in vec3 Normal;
in vec3 FragPos;

out vec4 colour;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

struct Light
{
	vec3 colour;
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
	vec4 ambientColour = vec4(light.colour, 1.0f) * light.ambientIntensity;

	float diffuseFactor = max(dot(normalize(Normal) , normalize(direction)), 0.0f);
	vec4 diffuseColour = vec4(light.colour, 1.0f) * light.diffuseIntensity * diffuseFactor;

	vec4 specularColour = vec4(0,0,0,0);

	if(diffuseFactor > 0.0f)
	{
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));

		float specularFactor = dot(fragToEye, reflectedVertex);
		if (specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColour = vec4(light.colour * material.specularIntensity * specularFactor, 1.0f);
		}
	}
	return (ambientColour + diffuseColour + specularColour);
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

		vec4 colour = CalcLightByDirection(pLight.base, direction);
		float attenuation = pLight.exponent * distance * distance +
							pLight.linear * distance +
							pLight.constant;
		return (colour / attenuation);
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
		vec4 colour = CalcPointLight(sLight.base);
		return colour * (1.0f - (1.0f - slFactor)*(1.0f / (1.0f - sLight.edge)));
	}
	else
	{
		return vec4(0,0,0,0);
	}
}

vec4 CalcSpotLights()
{
	vec4 totalColour = vec4(0,0,0,0);
	 for(int i=0;i<spotLightCount;i++)
	 {
		totalColour += CalcSpotLight(spotLights[i]);
	 }
	return totalColour;
}

vec4 CalcPointLights()
{
	 vec4 totalColour = vec4(0,0,0,0);
	 for(int i=0;i<pointLightCount;i++)
	 {
		totalColour += CalcPointLight(pointLights[i]);
	 }
	return totalColour;
}

void main()
{
	vec4 finalColour = CalcDirectionalLight();
	finalColour += CalcPointLights();
	finalColour += CalcSpotLights();
    // 10% ambient light in case there is not enough light
    finalColour += vec4(0.1, 0.1, 0.1, 0.0);

	colour = texture(theTexture,texCoord0) * finalColour;
    //uncomment to check normals
    //colour = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);

}
