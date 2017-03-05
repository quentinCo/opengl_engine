#version 430

#define MAX_TEX 10

//uniform int uNbCompositingTextures;

uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;

out vec4 fColor;

void main()
{
	fColor = vec4(0);
	fColor += texelFetch(uLayer0 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer1 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer2 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer3 , ivec2(gl_FragCoord.xy), 0);
}