#version 330

layout(location = 0) in vec3 aPosition;

uniform mat4 uModelViewProjMatrix;

void main()
{
	gl_Position = uModelViewProjMatrix * vec4(aPosition, 1);
}