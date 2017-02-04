#version 330

out vec3 fColor;

uniform sampler2D uScreenTexture;

void main()
{
	fColor = vec3(texelFetch(uScreenTexture, ivec2(gl_FragCoord.xy), 0));
}