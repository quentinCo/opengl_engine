#version 430

#define TILE_SIZE 32 //16

//#define FILTER_SIZE 25
#define FILTER_SIZE 15

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D blurredImage;

//uniform float blurFilter[FILTER_SIZE] = float[](0.0398942, 0.0396953, 0.0391043, 0.0381388, 0.036827, 0.0352065, 0.0333225, 0.0312254, 0.0289692, 0.0266085, 0.0241971, 0.0217852, 0.0194186, 0.0171369, 0.0149727, 0.0129518, 0.0110921, 0.0094049, 0.007895, 0.0065615816, 0.0053991, 0.0043986, 0.0035475, 0.0028327, 0.0022395);
uniform float blurFilter[FILTER_SIZE] = float[](0.1595769122, 0.1564170776, 0.1473080561, 0.1332898412, 0.1158766211, 0.0967882898, 0.077674422, 0.0598909863, 0.0443683339, 0.0315800633, 0.0215963866, 0.0141898371, 0.0089578121, 0.0054331877, 0.0031661806);

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
			for(int i = 1; i < FILTER_SIZE; ++i)
			{
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x + i, pixelGlobalPosition.y), 0) * blurFilter[i];				
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x - i, pixelGlobalPosition.y), 0) * blurFilter[i];
			}
		}
		else if(uDirection == 1)
		{
			for(int i = 1; i < FILTER_SIZE; ++i)
			{
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x, pixelGlobalPosition.y + i), 0) * blurFilter[i];
				fColor += texelFetch(uInitTex, ivec2(pixelGlobalPosition.x, pixelGlobalPosition.y - i), 0) * blurFilter[i];
			}
		}

		imageStore(blurredImage, pixelGlobalPosition, fColor);
	}
}
