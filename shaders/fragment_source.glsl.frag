#version 330

in vec4 vCol;
in vec2 texCoord0;
// flat in vec3 Normal;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

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

void main()
{
	vec4 finalColor = CalcDirectionalLight();
	finalColor += CalcPointLights();
	finalColor += CalcSpotLights();
    // 10% ambient light in case there is not enough light
    //finalColor += vec4(0.1, 0.1, 0.1, 0.0);

	color = texture(theTexture,texCoord0) * finalColor;
    //uncomment to check normals
    // color = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);

}
