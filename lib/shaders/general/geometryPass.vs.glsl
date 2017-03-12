#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBiTangent;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;
out mat3 vTbnMat;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

void main()
{
    vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
	vViewSpaceNormal = vec3(uNormalMatrix * vec4(aNormal, 0));
	vTexCoords = aTexCoords;

	vec3 tangent = normalize(vec3(uModelViewMatrix * vec4(aTangent, 0)));
	vec3 bitangent = normalize(vec3(uModelViewMatrix * vec4(aBiTangent, 0)));
	vec3 normal = normalize(vec3(uModelViewMatrix * vec4(aNormal, 0)));
	vTbnMat = mat3(tangent, bitangent, normal);

    gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
}