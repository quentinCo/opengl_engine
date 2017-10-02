#version 430

// Efficient blur
// Inspired by http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
// With gaussian sigma = 2.5

#define TILE_SIZE 32 //16

#define FILTER_SIZE 4

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D blurredImage;

//uniform float blurFilter[FILTER_SIZE] = float[](0.0797884561, 0.0782085388, 0.0736540281, 0.0666449206, 0.0579383106, 0.0483941449, 0.038837211, 0.0299454931, 0.0221841669, 0.0157900317, 0.0107981933, 0.0070949186, 0.0044789061, 0.0027165938, 0.0015830903);

uniform float weight[FILTER_SIZE] = float[](0.2929068271, 0.4830810905, 0.2240120824, 0.0560828837);
uniform float offset[FILTER_SIZE] = float[](0, 1.4402863507, 3.3635474597, 5.2931777789);

uniform sampler2D uInitTex;
uniform vec2 uWindowDim;
uniform int uDirection;

void main()
{
	ivec2 pixelGlobalPosition = ivec2(gl_GlobalInvocationID.xy);

	if(pixelGlobalPosition.x < uWindowDim.x && pixelGlobalPosition.y < uWindowDim.y)
	{
		vec4 fColor = texture2D(uInitTex, pixelGlobalPosition / uWindowDim, 0) * weight[0];
		for(int i = 1; i < FILTER_SIZE; ++i)
		{
			fColor += texture2D(uInitTex, vec2(pixelGlobalPosition.x, pixelGlobalPosition.y + offset[i]) / uWindowDim, 0) * weight[i];				
			fColor += texture2D(uInitTex, vec2(pixelGlobalPosition.x, pixelGlobalPosition.y - offset[i]) / uWindowDim, 0) * weight[i];
		}
		imageStore(blurredImage, pixelGlobalPosition, fColor);
	}
}
