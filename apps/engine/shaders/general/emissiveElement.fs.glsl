#version 330

layout(location = 0) out vec3 fEmissive;

//in vec3 viewSpacePosition;

uniform vec3 uKe;

void main()
{
	fEmissive = uKe;
}