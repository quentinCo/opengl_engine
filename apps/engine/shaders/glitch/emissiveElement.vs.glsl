#version 330

layout(location = 0) in vec3 aPosition;

uniform mat4 uModelViewProjMatrix;
uniform sampler2D uDepthMap;

void main()
{
	vec4 position = uModelViewProjMatrix * vec4(aPosition, 1);
	if(position.z < texture(uDepthMap, position.xy).x)
		gl_Position = uModelViewProjMatrix * vec4(aPosition, 1);

}