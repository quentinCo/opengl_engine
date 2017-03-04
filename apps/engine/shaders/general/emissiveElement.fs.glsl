#version 330

layout(location = 0) out vec3 fEmissive;

uniform vec3 uKe;
uniform sampler2D uDepthMap;

void main()
{
	if(gl_FragCoord.z < texture(uDepthMap, gl_FragCoord.xy).x)
		fEmissive = uKe;
	else
		fEmissive = vec3(0);
}