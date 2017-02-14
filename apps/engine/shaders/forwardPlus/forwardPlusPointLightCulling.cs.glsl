#version 430

#define MAX_LIGHTS 200
#define TILE_SIZE 32

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;

struct PointLight
{
	vec4 position;
	vec3 color;
	float intensity;
	float radiusAttenuation;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

layout(std430, binding = 0) readonly buffer uPointLights
{
	PointLight pointLights[ ];
};
uniform int uPointLightsNumber;

layout(std430, binding = 1) writeonly buffer uPointLightsIndex
{
	uint pointLightsIndex[ ];
};

uniform mat4 uInverseProjMatrix;
uniform vec2 uWindowDim;

uniform sampler2D uDepthMap;

shared int tilePointLightsIndex [MAX_LIGHTS];
shared int countTilePointLight;
//shared vec4 frustumPlan[6];
shared vec4 frustumPlans[4];
shared uint depthMin;
shared uint depthMax;

void initFrustum(vec2 pixelGlobalPosition, vec2 pixelLocalPosition, vec2 tilePosition)
{
	vec4 points[4];
	// Screen space
	points[0] = vec4(pixelGlobalPosition, depthMin, 1);
	points[1] = vec4(pixelLocalPosition.x + 1 + tilePosition.x * TILE_SIZE, pixelGlobalPosition.y, depthMin, 1);
	points[2] = vec4(pixelLocalPosition.y + 1 + tilePosition.y * TILE_SIZE, pixelLocalPosition.y + 1 + tilePosition.y * TILE_SIZE, depthMin, 1);
	points[3] = vec4(pixelGlobalPosition.x, pixelLocalPosition.y + 1 + tilePosition.y * TILE_SIZE, depthMin, 1);

	// Clip space
	for(int i = 0; i < 4; ++i)
	{
		points[i] = vec4(-1 + 2 * points[i].x / uWindowDim.x, -1 + 2 * (1 - points[i].y) / uWindowDim.y, depthMin, 1);
	}

	// Camera space
	for(int i = 0; i < 4; ++i)
	{
		points[i] = uInverseProjMatrix * points[i];
		points[i] /= points[i] .w;
	}

	// Compute frustums plan
	for(int i = 0; i < 4; ++i)
	{
		vec3 normal = normalize(cross(points[i % 4].xyz, points[(i + 1) % 4].xyz));
		float d = dot(normal, points[i].xyz);
		frustumPlans[i] = vec4(normal, d);
	}
}

void lightCulling(int threadIndex)
{
	int lightsToHandle = int(ceil(uPointLightsNumber / float(TILE_SIZE * TILE_SIZE)));

	for(int i = 0; i < lightsToHandle; ++i)
	{
		int lightIndex = threadIndex + i * TILE_SIZE * TILE_SIZE;
		if(lightIndex >= uPointLightsNumber)
		{
			break;
		}

		// Check depth
		if(pointLights[lightIndex].position.z - pointLights[lightIndex].radiusAttenuation > depthMin ||
		 pointLights[lightIndex].position.z + pointLights[lightIndex].radiusAttenuation < depthMax)
		{
			continue;	
		}

		// Check distance from pointLight to frustum
		bool intersect = false;
		for(int j = 0; j < 4; ++j)
		{
			// distance = dot(v, n) / length(n) -> length(n) = 1;
			//!\ CAUTION : possible bug here
			if(abs(dot(pointLights[lightIndex].position.xyz, frustumPlans[j].xyz)) - frustumPlans[j].w < pointLights[lightIndex].radiusAttenuation)
			{
				intersect = true;
				break;
			}
		}

		// Add light index if it's close enough to the frusum
		if(intersect && countTilePointLight < MAX_LIGHTS)
		{
			int index = atomicAdd(countTilePointLight, 1);
			tilePointLightsIndex[index] = lightIndex;
		}
	}
}

void main()
{
	vec2 pixelGlobalPosition = gl_GlobalInvocationID.xy;
	vec2 pixelLocalPosition = gl_LocalInvocationID.xy;
	vec2 tilePosition = gl_WorkGroupID.xy;	
	vec2 numTile = gl_NumWorkGroups.xy;
	int threadIndex = int(pixelLocalPosition.x * TILE_SIZE + pixelLocalPosition.y);

	if(threadIndex == 0)
	{
		countTilePointLight = 0;
		depthMin = 0xFFFFFFFF;
		depthMax = 0;

		for(int i = 0; i < MAX_LIGHTS; ++i)
		{
			tilePointLightsIndex[i] = 0;
		}
	}
	barrier();

	if(pixelGlobalPosition.x < uWindowDim.x && pixelGlobalPosition.y < uWindowDim.y)
	{
		uint currentDepth = floatBitsToUint(texelFetch(uDepthMap, ivec2(pixelGlobalPosition), 0).x);
		atomicMin(depthMin, currentDepth);
		atomicMax(depthMax, currentDepth);
	}
	barrier();

	if(threadIndex == 0)
	{
		initFrustum(pixelGlobalPosition, pixelLocalPosition, tilePosition);
	}
	barrier();

	lightCulling(threadIndex);
	
	barrier();
	if(threadIndex == 0)
	{
		int offset = int(tilePosition.x + tilePosition.y * numTile.x) * MAX_LIGHTS;
		int i = 0;
		for(i = 0; i < countTilePointLight; ++i)
		{
			pointLightsIndex[i + offset] = tilePointLightsIndex[i];
		}
		if(countTilePointLight <= MAX_LIGHTS)
		{
			pointLightsIndex[i + offset + 1] = -1; // To mark limite data.
		}
	}

}