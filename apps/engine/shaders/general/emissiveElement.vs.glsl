#version 330

layout(location = 0) in vec3 aPosition;

out vec3 viewSpacePosition;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;

void main()
{
	viewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1));
	gl_Position = uModelViewProjMatrix * vec4(aPosition, 1);
}