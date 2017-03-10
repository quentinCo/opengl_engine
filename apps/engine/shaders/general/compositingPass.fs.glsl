#version 430

//#define MAX_TEX 10

//uniform int uNbCompositingTextures;

uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;

uniform vec2 uLayer0Size;

out vec4 fColor;

void main()
{
	fColor = vec4(0);
	fColor += texture2D(uLayer0 , vec2(gl_FragCoord.xy) / uLayer0Size, 0);
	fColor += texture2D(uLayer1 , vec2(gl_FragCoord.xy) / uLayer0Size, 0);
	fColor += texture2D(uLayer2 , vec2(gl_FragCoord.xy) / uLayer0Size, 0);
	fColor += texture2D(uLayer3 , vec2(gl_FragCoord.xy) / uLayer0Size, 0);
}