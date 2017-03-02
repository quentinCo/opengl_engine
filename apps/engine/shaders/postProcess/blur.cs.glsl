#version 430

#define TILE_SIZE 32 //16
#define FILTER_SIZE 51
//#define FILTER_SIZE 101

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D blurredImage;


//layout(std430, binding = 0) readonly buffer uBlurFilter
//{
//	float blurFilter[][];
//};
//uniform int uFilterSize;

uniform sampler2D uInitTex;
uniform vec2 uWindowDim;
uniform int uDirection;

shared float[FILTER_SIZE] blurFilter;
//shared int filterSize;

void main()
{
	ivec2 pixelGlobalPosition = ivec2(gl_GlobalInvocationID.xy);
	vec2 pixelLocalPosition = gl_LocalInvocationID.xy;
	int threadIndex = int(pixelLocalPosition.x + pixelLocalPosition.y * TILE_SIZE );

	if(threadIndex == 0)
	{
		//int sigma = 50;
		int sigma = 10;
		for(int i = 0; i < FILTER_SIZE; ++i)
			blurFilter[i] = exp(-(i * i) / (2 * sigma * sigma)) / sqrt(2 * 3.14 * sigma * sigma);
	}
	barrier();
	if(pixelGlobalPosition.x < uWindowDim.x && pixelGlobalPosition.y < uWindowDim.y)
	{
		vec4 fColor = vec4(0);
		if(uDirection == 0)
		{
			for(int i = int(-FILTER_SIZE / 2.f); i < int(FILTER_SIZE / 2.f); ++i)
			{
				int index = int(min(uWindowDim.x - 1, max(0, pixelGlobalPosition.x + i)));
				fColor += texelFetch(uInitTex, ivec2(index, pixelGlobalPosition.y), 0) * blurFilter[i + int(FILTER_SIZE / 2.f)];
			}
		}
		else if(uDirection == 1)
		{
			for(int i = int(-FILTER_SIZE / 2.f); i < int(FILTER_SIZE / 2.f); ++i)
			{
				int index = int(min(uWindowDim.y - 1, max(0, pixelGlobalPosition.y + i)));
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x, index), 0) * blurFilter[i + int(FILTER_SIZE / 2.f)];
			}
		}
		
		imageStore(blurredImage, pixelGlobalPosition, fColor);
	}
}
