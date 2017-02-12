#version 430
#define MAX_LIGHTS 10 // TODO : revoir

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

struct Light
{
	vec4 position; // vec4 necessary cause std140 gap on vec4. If vec3, memory offset -> bug 
	vec3 color;
	float intensity;
};

layout(std140, binding = 1) uniform uDirectionalLights
{
	Light directionalLights[MAX_LIGHTS];
};
uniform int uDirectionalLightsNumber;

layout(std140, binding = 2) uniform uPointLights
{
	Light pointLights[MAX_LIGHTS];
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
void main()
{

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if(pixelCoords.x < uWindowDim.x && pixelCoords.y < uWindowDim.y)
    {
        vec3 position = vec3(texelFetch(uGPosition, pixelCoords, 0)); // Correspond a vViewSpacePosition dans le forward renderer
		
        vec3 ka = vec3(texelFetch(uGAmbient, pixelCoords, 0)); 
        vec3 kd = vec3(texelFetch(uGDiffuse, pixelCoords, 0));
        vec3 ks = vec3(texelFetch(uGlossyShininess, pixelCoords, 0)).xyz;
        float shininess = (texelFetch(uGlossyShininess, pixelCoords, 0)).w;

        vec3 normal = vec3(texelFetch(uGNormal, pixelCoords, 0));
        vec3 eyeDir = normalize(-position);

		int i = 0;
			vec3 lightDirection = (uViewMatrix * vec4(normalize(directionalLights[i].position))).xyz;
			vec3 directionalLightsIntensity = directionalLights[i].color * directionalLights[i].intensity;	

			vec3 hDirLight = normalize(eyeDir + lightDirection);
			float dothDirLight = (shininess == 0) ? 1.f :max(0.f, dot(normal, hDirLight)); // TODO : revoir

		i = 0;
			vec3 lightPosition = (uViewMatrix * vec4(pointLights[i].position)).xyz;
			vec3 pointLighIntensity = pointLights[i].color * pointLights[i].intensity;
		
			float distToPointLight = length(lightPosition - position);
			vec3 dirToPointLight = (lightPosition - position) / distToPointLight;
			vec3 pointLightsIncidentLight = pointLighIntensity / (distToPointLight * distToPointLight); // TODO : revoir

			vec3 hPointLight = normalize(eyeDir + dirToPointLight);

			float dothPointLight = (shininess == 0) ? 1.f : max(0.f, dot(normal, hPointLight));

        vec3 fColor = vec3(0);
        fColor += ka;
        fColor += kd * (directionalLightsIntensity * max(0.f, dot(normal, lightDirection)) + pointLightsIncidentLight * max(0., dot(normal, dirToPointLight)));
        fColor += ks * (directionalLightsIntensity * dothDirLight + pointLightsIncidentLight * dothPointLight);

        imageStore(imgOutput, pixelCoords, vec4(fColor,1));
    }
    else
    {
        imageStore(imgOutput, pixelCoords, vec4(0, normalize(pixelCoords),1));
    }
}