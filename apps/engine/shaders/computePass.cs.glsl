#version 430

layout(local_size_x = 32, local_size_y = 32) in;
//layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;

uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

// GBuffer Textures
uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec2 uWindowsDim;

void main()
{

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if(pixelCoords.x < uWindowsDim.x && pixelCoords.y < uWindowsDim.y)
    {
        vec3 position = vec3(texelFetch(uGPosition, pixelCoords, 0)); // Correspond a vViewSpacePosition dans le forward renderer

        vec3 ka = vec3(texelFetch(uGAmbient, pixelCoords, 0)); 
        vec3 kd = vec3(texelFetch(uGDiffuse, pixelCoords, 0));
        vec3 ks = vec3(texelFetch(uGlossyShininess, pixelCoords, 0)).xyz;
        float shininess = (texelFetch(uGlossyShininess, pixelCoords, 0)).w;

        vec3 normal = vec3(texelFetch(uGNormal, pixelCoords, 0));
        vec3 eyeDir = normalize(-position);

        float distToPointLight = length(uPointLightPosition - position);
        vec3 dirToPointLight = (uPointLightPosition - position) / distToPointLight;
        vec3 pointLightIncidentLight = uPointLightIntensity / (distToPointLight * distToPointLight);

        // half vectors, for blinn-phong shading
        vec3 hPointLight = normalize(eyeDir + dirToPointLight);
        vec3 hDirLight = normalize(eyeDir + uDirectionalLightDir);

        float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
        float dothDirLight = shininess == 0 ? 1.f :max(0.f, dot(normal, hDirLight));

        if (shininess != 1.f && shininess != 0.f)
        {
            dothPointLight = pow(dothPointLight, shininess);
            dothDirLight = pow(dothDirLight, shininess);
        }

        vec3 fColor = vec3(0);
        fColor += ka;
        fColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
        fColor += ks * (uDirectionalLightIntensity * dothDirLight + pointLightIncidentLight * dothPointLight);

        imageStore(imgOutput, pixelCoords, vec4(fColor,1));
    }
    else
    {
        imageStore(imgOutput, pixelCoords, vec4(0, normalize(pixelCoords),1));
    }
}