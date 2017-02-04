#version 330

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

void main()
{

    vec3 ka = uKa * vec3(texture(uKaSampler, vTexCoords));  // ambient
    vec3 kd = uKd * vec3(texture(uKdSampler, vTexCoords));  // diffuse
    vec3 ks = uKs * vec3(texture(uKsSampler, vTexCoords));  // specular

    float shininess = uShininess * vec3(texture(uShininessSampler, vTexCoords)).x;

    vec3 normal = normalize(vViewSpaceNormal);
   // vec3 eyeDir = normalize(-vViewSpacePosition);

    fPosition = vViewSpacePosition;
    fNormal = normal;
    fAmbient = ka;
    fDiffuse = kd;
    fGlossyShininess = vec4(ks, shininess);
}