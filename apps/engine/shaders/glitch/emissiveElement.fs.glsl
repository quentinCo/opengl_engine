#version 330

layout(location = 0) out vec3 fEmissive;

uniform vec3 uKe;

void main()
{
	fEmissive = uKe;
}