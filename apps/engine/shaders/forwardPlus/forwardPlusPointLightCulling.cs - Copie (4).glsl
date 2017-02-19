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

layout(std430, binding = 2) writeonly buffer uDebugOutput
{
	float debugOutput[];
};

uniform mat4 uInverseProjMatrix;
uniform mat4 uViewProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;
uniform vec2 uWindowDim;

uniform sampler2D uDepthMap;

shared int tilePointLightsIndex [MAX_LIGHTS];
shared int countTilePointLight;
//shared vec4 frustumPlan[6];
shared vec4 frustumPlans[6];
shared uint depthMinInt;
shared uint depthMaxInt;

void initFrustum(vec2 pixelGlobalPosition, vec2 pixelLocalPosition, vec2 tilePosition, vec2 numTiles, int tileIndex)
{
	frustumPlans[0] = uViewProjMatrix[0] + uViewProjMatrix[3];
	frustumPlans[1] = -uViewProjMatrix[0] + uViewProjMatrix[3];
	frustumPlans[2] = uViewProjMatrix[1] + uViewProjMatrix[3];
	frustumPlans[3] = -uViewProjMatrix[1] + uViewProjMatrix[3];
	frustumPlans[4] = uViewProjMatrix[2] + uViewProjMatrix[3];
	frustumPlans[5] = -uViewProjMatrix[2] + uViewProjMatrix[3];

	
	int offset = int(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * 24;
	int id;
	for(id = 0; id < 6; ++id)
	{
		debugOutput[id * 4 + offset] = frustumPlans[id].x;
		debugOutput[id * 4 + offset + 1] = frustumPlans[id].y;
		debugOutput[id * 4 + offset + 2] = frustumPlans[id].z;
		debugOutput[id * 4 + offset + 3] = frustumPlans[id].w;
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
		vec3 pointLightCoords = (uViewMatrix * pointLights[lightIndex].position).xyz;

		// Check depth
		//debugOutput[lightIndex * 4 + i] = pointLights[lightIndex].radiusAttenuation;		
		//debugOutput[lightIndex * 4 + i + 1] = pointLightCoords.x;
		//debugOutput[lightIndex * 4 + i + 2] = pointLightCoords.y;
		//debugOutput[lightIndex * 4 + i + 3] = pointLightCoords.z;

		if(pointLightCoords.z - pointLights[lightIndex].radiusAttenuation > frustumPlans[4].w ||
		 pointLightCoords.z + pointLights[lightIndex].radiusAttenuation < frustumPlans[5].w)
		{
			continue;	
		}

		// Check distance from pointLight to frustum
		bool intersect = false;
		for(int j = 0; j < 4; ++j)
		{
			// distance = dot(v, n) / length(n) -> length(n) = 1;
			//!\ CAUTION : possible bug here
			if(abs(dot(pointLightCoords.xyz, frustumPlans[j].xyz)) - frustumPlans[j].w < pointLights[lightIndex].radiusAttenuation)
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
	vec2 numTiles = gl_NumWorkGroups.xy;
	int threadIndex = int(pixelLocalPosition.x * TILE_SIZE + pixelLocalPosition.y);
	
	int tileIndex = int(tilePosition.x + tilePosition.y * numTiles.x);

	if(threadIndex == 0)
	{
		countTilePointLight = 0;
		depthMinInt = 0xFFFFFFFF;
		depthMaxInt = 0;

		for(int i = 0; i < MAX_LIGHTS; ++i)
		{
			tilePointLightsIndex[i] = 0;
		}
	}
	barrier();

	if(pixelGlobalPosition.x < uWindowDim.x && pixelGlobalPosition.y < uWindowDim.y)
	{
		uint currentDepth = floatBitsToUint(texelFetch(uDepthMap, ivec2(pixelGlobalPosition), 0).x);
		
		atomicMin(depthMinInt, currentDepth);
		atomicMax(depthMaxInt, currentDepth);
	}
	barrier();
	
	if(threadIndex == 0)
	{
		float depthMin = uintBitsToFloat(depthMinInt);
		float depthMax = uintBitsToFloat(depthMaxInt);

		//Convert Clip depth to View depth
		depthMin = (0.5 * uProjMatrix[3][2]) / (depthMin + 0.5 * uProjMatrix[2][2] - 0.5);
		depthMax = (0.5 * uProjMatrix[3][2]) / (depthMax + 0.5 * uProjMatrix[2][2] - 0.5);
		
		frustumPlans[4] = uViewMatrix * vec4(0,0,-1,-depthMin);
		frustumPlans[4] /= length(frustumPlans[4].xyz);

		frustumPlans[5] = uViewMatrix * vec4(0,0,1,depthMax);
		frustumPlans[5] /= length(frustumPlans[5].xyz);

		initFrustum(pixelGlobalPosition, pixelLocalPosition, tilePosition, numTiles, tileIndex);
	}
	barrier();

	lightCulling(threadIndex);
	
	barrier();
	if(threadIndex == 0)
	{
		int offset = int(tilePosition.x + tilePosition.y * numTiles.x) * MAX_LIGHTS;
		int i = 0;
		for(i = 0; i < countTilePointLight; ++i)
		{
			pointLightsIndex[i + offset] = tilePointLightsIndex[i];
		}
		if(countTilePointLight <= MAX_LIGHTS)
		{
			int indexMinus1 = i + offset;
			if(countTilePointLight != 0)
				indexMinus1 += 1; 
			pointLightsIndex[indexMinus1] = -1; // To mark limite data.
		}
	}

}