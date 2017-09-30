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
in mat3 vTbnMat;

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
uniform sampler2D uNormalSampler;

int computePointLighting(vec3 position, vec3 ka, vec3 kd, vec3 ks, float shininess, vec3 normal, vec3 eyeDir)
{
	int count = 0;
	// pointLight Tile in the lightCulling
	int pointLightListIndex = ((int(gl_FragCoord.x / TILE_SIZE) + int(gl_FragCoord.y / TILE_SIZE) * int(ceil(uWindowDim.x / TILE_SIZE))) *  MAX_LIGHTS);
	
	vec3 diffusePointLightIntensity = vec3(0);
	vec3 specularPointLightIntensity = vec3(0);

	for(int i = 0; i < MAX_LIGHTS && i < uPointLightsNumber && pointLightsIndex[pointLightListIndex + i] != -1; ++i)
	{
		int pointLightIndex = pointLightsIndex[pointLightListIndex + i];
		count++;

		vec3 lightCoords = (uViewMatrix * pointLights[pointLightIndex].position).xyz;
		float distToPointLight = length(lightCoords - position);

		// Attenuation compute https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/
		float attenuation = 1 / (pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * distToPointLight + pointLights[pointLightIndex].quadraticAttenuation * distToPointLight * distToPointLight);
		attenuation -= 1 /( pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * pointLights[pointLightIndex].radiusAttenuation + pointLights[pointLightIndex].quadraticAttenuation *  pointLights[pointLightIndex].radiusAttenuation *  pointLights[pointLightIndex].radiusAttenuation);
		
		vec3 lightIntensity = (distToPointLight <= pointLights[pointLightIndex].radiusAttenuation) ? (pointLights[pointLightIndex].color * pointLights[pointLightIndex].intensity) * attenuation : vec3(0);
		
		vec3 dirToPointLight = (lightCoords - position) / distToPointLight;
		diffusePointLightIntensity += lightIntensity * max(0., dot(normal, dirToPointLight));

		float dothLight = (shininess == 0) ? 1.f : max(0.f, dot(normal, normalize(eyeDir + dirToPointLight)));
		if (shininess != 1.f && shininess != 0.f)
			dothLight = pow(dothLight, shininess);

		specularPointLightIntensity += lightIntensity * dothLight;
	}

	fColor += kd * diffusePointLightIntensity;
	fColor += ks * specularPointLightIntensity;

	return count;
}


void computeDirectionalLighting(vec3 position, vec3 ka, vec3 kd, vec3 ks, float shininess, vec3 normal, vec3 eyeDir)
{
	vec3 diffuseDirectionalLightIntensity = vec3(0);
	vec3 specularDirectionalLightIntensity = vec3(0);

	for(int i = 0; i < uDirectionalLightsNumber; ++i)
	{
		vec3 lightCoords = (uViewMatrix * normalize(directionalLights[i].position)).xyz;
		vec3 lightIntensity = directionalLights[i].color * directionalLights[i].intensity;
		diffuseDirectionalLightIntensity += lightIntensity * max(0.f, dot(normal, lightCoords));

		float dothLight = (shininess == 0) ? 1.f :max(0.f, dot(normal, normalize(eyeDir + lightCoords))); 
		if (shininess != 1.f && shininess != 0.f)
			dothLight = pow(dothLight, shininess);

		specularDirectionalLightIntensity += lightIntensity * dothLight;
	}

	
	fColor += kd * diffuseDirectionalLightIntensity;
	fColor += ks * specularDirectionalLightIntensity;
}

vec3 getNormal()
{
	vec3 normal = vec3(texture(uNormalSampler, vTexCoords));

	if(normal != vec3(0))
	{
		normal = (normal * 2 - 1);
		normal = normalize(vTbnMat * normal);
	}
	else
	{
		normal = vViewSpaceNormal;
	}

	return normal;
}

void computeFragColor()
{
    vec3 position = vViewSpacePosition;

	vec3 ka = uKa * vec3(texture(uKaSampler, vTexCoords));
    vec3 kd = uKd * vec3(texture(uKdSampler, vTexCoords));
    vec3 ks = uKs * vec3(texture(uKsSampler, vTexCoords));

    float shininess = uShininess * vec3(texture(uShininessSampler, vTexCoords)).x;

    vec3 normal = getNormal();
    vec3 eyeDir = normalize(-position);

	fColor = vec3(0);
    fColor += 0.05 * ka;

	if(uDirectionalLightsNumber > 0)
		computeDirectionalLighting(position, ka, kd, ks, shininess, normal, eyeDir);

	int count = 0;
	if(uPointLightsNumber > 0)
		count = computePointLighting(position, ka, kd, ks, shininess, normal, eyeDir);

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

}
void main()
{
	computeFragColor();
	fEmissive = vec3(0);
}