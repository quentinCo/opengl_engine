#version 430

out vec3 fColor;

uniform sampler2D uDepthMap;

void main()
{
	float depth = texelFetch(uDepthMap, ivec2(gl_FragCoord.xy), 0).x;
	fColor = vec3(depth);
}