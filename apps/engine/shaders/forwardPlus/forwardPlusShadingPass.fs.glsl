#version 430

#define MAX_LIGHTS 200
#define TILE_SIZE 16

layout(location = 0) out vec3 fColor;
layout(location = 1) out vec3 fEmissive;

struct Light
{
	vec4 position; // vec4 necessary cause std140 gap on vec4. If vec3, memory offset -> bug 
	vec3 color;
	float intensity;
};

struct PointLight
{
	vec4 position;
	vec3 color;
	float intensity;
	float radiusAttenuation;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

layout(std430, binding = 1) readonly buffer uDirectionalLights
{
	Light directionalLights[ ];
};
uniform int uDirectionalLightsNumber;

layout(std430, binding = 2) readonly buffer uPointLights
{
	PointLight pointLights[ ];
};
uniform int uPointLightsNumber;

layout(std430, binding = 3) readonly buffer uPointLightsIndex
{
	int pointLightsIndex[ ];
};

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform mat4 uModelViewMatrix;
uniform mat4 uViewMatrix;

uniform vec2 uWindowDim;

//  Color
uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

vec3 computeFragColor()
{
    vec3 position = vViewSpacePosition;

	vec3 ka = uKa * vec3(texture(uKaSampler, vTexCoords));
    vec3 kd = uKd * vec3(texture(uKdSampler, vTexCoords));
    vec3 ks = uKs * vec3(texture(uKsSampler, vTexCoords));

    float shininess = uShininess * vec3(texture(uShininessSampler, vTexCoords)).x;

    vec3 normal = vViewSpaceNormal;
    vec3 eyeDir = normalize(-position);

	vec3 diffuseDirectionalLightIntensity = vec3(0);
	vec3 specularDirectionalLightIntensity = vec3(0);

	vec3 lightCoords = vec3(0);
	vec3 lightIntensity = vec3(0);
	vec3 hLight = vec3(0);
	float dothLight = 0;
	for(int i = 0; i < uDirectionalLightsNumber; ++i)
	{
		lightCoords = (uViewMatrix * vec4(normalize(directionalLights[i].position))).xyz;
		lightIntensity = directionalLights[i].color * directionalLights[i].intensity;
		diffuseDirectionalLightIntensity += lightIntensity * max(0.f, dot(normal, lightCoords));

		dothLight = (shininess == 0) ? 1.f :max(0.f, dot(normal, normalize(eyeDir + lightCoords))); 
		if (shininess != 1.f && shininess != 0.f)
			dothLight = pow(dothLight, shininess);

		specularDirectionalLightIntensity += lightIntensity * dothLight;
	}	


	int pointLightListIndex = ((int(gl_FragCoord.x / TILE_SIZE) + int(gl_FragCoord.y / TILE_SIZE) * int(ceil(uWindowDim.x / TILE_SIZE))) *  MAX_LIGHTS);
	//int count = 0;

	vec3 diffusePointLightIntensity = vec3(0);
	vec3 specularPointLightIntensity = vec3(0);

	float distToPointLight = 0;
	vec3 dirToPointLight = vec3(0);
	float attenuation = 0;
	for(int i = 0; i < MAX_LIGHTS && i < uPointLightsNumber && pointLightsIndex[pointLightListIndex + i] != -1; ++i)
	{
		int pointLightIndex = pointLightsIndex[pointLightListIndex + i];
		//count++;

		lightCoords = (uViewMatrix * vec4(pointLights[pointLightIndex].position)).xyz;
		distToPointLight = length(lightCoords - position);
		attenuation = 1 / (pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * distToPointLight + pointLights[pointLightIndex].quadraticAttenuation * distToPointLight * distToPointLight);
		attenuation -= 1 /( pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * pointLights[pointLightIndex].radiusAttenuation + pointLights[pointLightIndex].quadraticAttenuation *  pointLights[pointLightIndex].radiusAttenuation *  pointLights[pointLightIndex].radiusAttenuation);
		lightIntensity = (distToPointLight <= pointLights[pointLightIndex].radiusAttenuation) ? (pointLights[pointLightIndex].color * pointLights[pointLightIndex].intensity) * attenuation : vec3(0);
		
		dirToPointLight = (lightCoords - position) / distToPointLight;
		diffusePointLightIntensity += lightIntensity * max(0., dot(normal, dirToPointLight));

		dothLight = (shininess == 0) ? 1.f : max(0.f, dot(normal, normalize(eyeDir + dirToPointLight)));
		if (shininess != 1.f && shininess != 0.f)
			dothLight = pow(dothLight, shininess);

		specularPointLightIntensity += lightIntensity * dothLight;
	}

	vec3 fColor = vec3(0);
//    fColor += ka;
	fColor += kd * (diffuseDirectionalLightIntensity + diffusePointLightIntensity);
	fColor += ks * (specularDirectionalLightIntensity + specularPointLightIntensity);
	
	//if(count > 50)
	//{
	//	fColor += vec3(0.5, 0, 0);
	//}
	//else if(count > 25)
	//{
	//	fColor += vec3(0, 0.5, 0);
	//}
	//else if(count > 10)
	//{
	//	fColor += vec3(0, 0, 0.5);
	//}

	return fColor;
}
void main()
{
	fColor = computeFragColor();
	fEmissive = vec3(0);
}