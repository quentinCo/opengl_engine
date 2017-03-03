#version 430

#define TILE_SIZE 32 //16

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D blurredImage;

layout(std430, binding = 2) readonly buffer uBlurFilter
{
	float blurFilter[];
};
uniform int uFilterSize;

uniform sampler2D uInitTex;
uniform vec2 uWindowDim;
uniform int uDirection;

void main()
{
	ivec2 pixelGlobalPosition = ivec2(gl_GlobalInvocationID.xy);

	if(pixelGlobalPosition.x < uWindowDim.x && pixelGlobalPosition.y < uWindowDim.y)
	{
		vec4 fColor = texelFetch(uInitTex, pixelGlobalPosition, 0) * blurFilter[0];
		if(uDirection == 0)
		{
			for(int i = 1; i < uFilterSize; ++i)
			{
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x + i, pixelGlobalPosition.y), 0) * blurFilter[i];				
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x - i, pixelGlobalPosition.y), 0) * blurFilter[i];
			}
		}
		else if(uDirection == 1)
		{
			for(int i = 1; i < uFilterSize; ++i)
			{
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x, pixelGlobalPosition.y + i), 0) * blurFilter[i];
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x, pixelGlobalPosition.y - i), 0) * blurFilter[i];
			}
		}

		imageStore(blurredImage, pixelGlobalPosition, fColor);
	}
}