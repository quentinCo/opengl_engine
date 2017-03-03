#version 430

#define MAX_TEX 10

//uniform int uNbCompositingTextures;

uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;
uniform sampler2D uLayer4;
uniform sampler2D uLayer5;
uniform sampler2D uLayer6;
uniform sampler2D uLayer7;
uniform sampler2D uLayer8;
uniform sampler2D uLayer9;

out vec4 fColor;

void main()
{
	fColor = vec4(0);
	fColor += texelFetch(uLayer0 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer1 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer2 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer3 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer4 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer5 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer6 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer7 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer8 , ivec2(gl_FragCoord.xy), 0);
	fColor += texelFetch(uLayer9 , ivec2(gl_FragCoord.xy), 0);
}