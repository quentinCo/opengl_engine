#version 430

#define MAX_LIGHTS 200

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

out vec3 fColor;

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

		hLight = normalize(eyeDir + lightCoords);
		dothLight = (shininess == 0) ? 1.f :max(0.f, dot(normal, hLight)); 
		if (shininess != 1.f && shininess != 0.f)
		{
			dothLight = pow(dothLight, shininess);
		}
		specularDirectionalLightIntensity += lightIntensity * dothLight;
	}	


	int pointLightListIndex = ((int(gl_FragCoord.x / 32.f) + int(gl_FragCoord.y / 32.f) * int(ceil(uWindowDim.x / 32.f))) *  MAX_LIGHTS);
	int count = 0;

	vec3 diffusePointLightIntensity = vec3(0);
	vec3 specularPointLightIntensity = vec3(0);

	float distToPointLight = 0;
	vec3 dirToPointLight = vec3(0);
	float attenuation = 0;
	for(int i = 0; i < MAX_LIGHTS && i < uPointLightsNumber && pointLightsIndex[pointLightListIndex + i] != -1; ++i)
	{
		int pointLightIndex = pointLightsIndex[pointLightListIndex + i];
		count++;

		lightCoords = (uViewMatrix * vec4(pointLights[pointLightIndex].position)).xyz;
		distToPointLight = length(lightCoords - position);
		attenuation = 1 / (pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * distToPointLight + pointLights[pointLightIndex].quadraticAttenuation * distToPointLight * distToPointLight);
		attenuation -= 1 /( pointLights[pointLightIndex].constantAttenuation + pointLights[pointLightIndex].linearAttenuation * pointLights[pointLightIndex].radiusAttenuation + pointLights[pointLightIndex].quadraticAttenuation *  pointLights[pointLightIndex].radiusAttenuation *  pointLights[pointLightIndex].radiusAttenuation);
		lightIntensity = (distToPointLight <= pointLights[pointLightIndex].radiusAttenuation) ? (pointLights[pointLightIndex].color * pointLights[pointLightIndex].intensity) * attenuation : vec3(0);
		
		dirToPointLight = (lightCoords - position) / distToPointLight;
		diffusePointLightIntensity += lightIntensity * max(0., dot(normal, dirToPointLight));

		hLight = normalize(eyeDir + dirToPointLight);
		dothLight = (shininess == 0) ? 1.f : max(0.f, dot(normal, hLight));
		if (shininess != 1.f && shininess != 0.f)
		{
			dothLight = pow(dothLight, shininess);
		}
		specularPointLightIntensity += lightIntensity * dothLight;
	}

	vec3 fColor = vec3(0);
//    fColor += ka;
	fColor += kd * (diffuseDirectionalLightIntensity + diffusePointLightIntensity);
	fColor += ks * (specularDirectionalLightIntensity + specularPointLightIntensity);
	//if(mod(int(gl_FragCoord.x), 32) == 0 || mod(int(gl_FragCoord.y), 32) == 0)
	//{
	//	fColor = vec3(0.5);
	//}
	if(gl_FragCoord.x < 32 && gl_FragCoord.y < 32)
	{
		fColor += vec3(0.25);
	}
	//float x = (int(gl_FragCoord.x / 32.f) / ceil(uWindowDim.x / 32.f));// + (int(gl_FragCoord.y / 32.f) / int(ceil(uWindowDim.y / 32.f)))* int(ceil(uWindowDim.x / 32.f));
	//float y = (int(gl_FragCoord.y / 32.f) / ceil(uWindowDim.y / 32.f));
	//fColor = vec3(x , y, 0);
	//fColor += vec3(count/8.f, 0, 0);
	//fColor += vec3(pointLightListIndex / (ceil(uWindowDim.x / 32) * ceil(uWindowDim.y / 32) * MAX_LIGHTS));

	return fColor;
}
void main()
{
	fColor = computeFragColor();
}