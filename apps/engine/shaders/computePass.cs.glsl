#version 430
#define MAX_LIGHTS 10 // TODO : revoir

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float intensity;
	float phiAngle;
	float thetaAngle;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

layout(std140) uniform uDirectionalLights
{
	DirectionalLight directionalLights[MAX_LIGHTS];
};
uniform int uDirectionalLightsNumber;

layout(std140) uniform uPointLights
{
	PointLight pointLights[MAX_LIGHTS];
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

	float dothDirLight = 0;
	vec3 directionalLightsIntensity = vec3(0);
	vec3 globalDirection = vec3(0); // TODO : revoir
	for(int i = 1; i < uDirectionalLightsNumber; ++i)
	{
		vec3 lightDirection = (uViewMatrix * vec4(normalize(directionalLights[i].direction), 0)).xyz;
		directionalLightsIntensity += directionalLights[i].color * directionalLights[i].intensity; // TODO : revoir
		globalDirection += lightDirection;	

		vec3 hDirLight = normalize(eyeDir + lightDirection);
		dothDirLight += (shininess == 0) ? 1.f :max(0.f, dot(normal, hDirLight)); // TODO : revoir
	}

	float dothPointLight = 0;
	vec3 pointLightsIncidentLight = vec3(0);
	vec3 globalDirToPoint = vec3(0);
	for(int i = 0; i < uPointLightsNumber; ++i)
	{
		vec3 lightPosition = (uViewMatrix * vec4(pointLights[i].position, 1)).xyz;
		vec3 pointLighIntensity = pointLights[i].color * pointLights[i].intensity;
		
		float distToPointLight = length(lightPosition - position);
		vec3 dirToPointLight = (lightPosition - position) / distToPointLight;
		pointLightsIncidentLight += pointLighIntensity / (distToPointLight * distToPointLight); // TODO : revoir
		globalDirToPoint += dirToPointLight;

		vec3 hPointLight = normalize(eyeDir + dirToPointLight);

		dothPointLight += (shininess == 0) ? 1.f : max(0.f, dot(normal, hPointLight));
	}

    if (shininess != 1.f && shininess != 0.f)
    {
        dothPointLight = pow(dothPointLight, shininess);
        dothDirLight = pow(dothDirLight, shininess);
    }

	vec3 fColor = vec3(0);
    fColor += ka;
    fColor += kd * (directionalLightsIntensity * max(0.f, dot(normal, globalDirection)) + pointLightsIncidentLight * max(0., dot(normal, globalDirToPoint)));
    fColor += ks * (directionalLightsIntensity * dothDirLight + pointLightsIncidentLight * dothPointLight);
	
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