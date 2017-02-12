#version 430

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

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

layout(std430, binding = 1) buffer uDirectionalLights
{
	Light directionalLights[];
};
uniform int uDirectionalLightsNumber;

layout(std430, binding = 2) buffer uPointLights
{
	PointLight pointLights[];
};
uniform int uPointLightsNumber;

uniform mat4 uViewMatrix;

// GBuffer Textures
uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec2 uWindowDim;

vec3 computeFragColor(ivec2 pixelCoords)
{
    vec3 position = vec3(texelFetch(uGPosition, pixelCoords, 0)); // Correspond a vViewSpacePosition dans le forward renderer

    vec3 ka = vec3(texelFetch(uGAmbient, pixelCoords, 0)); 
    vec3 kd = vec3(texelFetch(uGDiffuse, pixelCoords, 0));
    vec3 ks = vec3(texelFetch(uGlossyShininess, pixelCoords, 0)).xyz;
    float shininess = (texelFetch(uGlossyShininess, pixelCoords, 0)).w;

    vec3 normal = vec3(texelFetch(uGNormal, pixelCoords, 0));
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

	vec3 diffusePointLightIntensity = vec3(0);
	vec3 specularPointLightIntensity = vec3(0);

	float distToPointLight = 0;
	vec3 dirToPointLight = vec3(0);
	float attenuation = 0;
	for(int i = 0; i < uPointLightsNumber; ++i)
	{
		lightCoords = (uViewMatrix * vec4(pointLights[i].position)).xyz;
		distToPointLight = length(lightCoords - position);
		dirToPointLight = (lightCoords - position) / distToPointLight;
		attenuation = ( pointLights[i].constantAttenuation + pointLights[i].linearAttenuation * distToPointLight + pointLights[i].quadraticAttenuation * distToPointLight * distToPointLight);
		lightIntensity = (pointLights[i].color * pointLights[i].intensity) / attenuation;
		
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
	
	return fColor;
}

void main()
{

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if(pixelCoords.x < uWindowDim.x && pixelCoords.y < uWindowDim.y)
    {
		vec3 fColor = computeFragColor(pixelCoords);

        imageStore(imgOutput, pixelCoords, vec4(fColor,1));
    }
    else
    {
        imageStore(imgOutput, pixelCoords, vec4(0, normalize(pixelCoords),1));
    }
}