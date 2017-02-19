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
	vec4 points[4];
	// Screen space
	points[0] = vec4(pixelGlobalPosition, -1, 1);
	points[1] = vec4((tilePosition.x + 1) * TILE_SIZE, pixelGlobalPosition.y, -1, 1);
	points[2] = vec4((tilePosition.x + 1) * TILE_SIZE, (tilePosition.y + 1) * TILE_SIZE, -1, 1);
	points[3] = vec4(pixelGlobalPosition.x, (tilePosition.y + 1) * TILE_SIZE, -1, 1);
	
	int offset = int(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * MAX_LIGHTS;
	int id;

	// Clip space
	for(int i = 0; i < 4; ++i)
	{
		points[i].x = points[i].x / (gl_NumWorkGroups.x * TILE_SIZE);
		points[i].y = points[i].y / (gl_NumWorkGroups.y * TILE_SIZE);
		points[i].xy = vec2((-1 + 2 * points[i].x), (-1 + 2 * (1 - points[i].y)));
	}
	
	// Camera space
	for(int i = 0; i < 4; ++i)
	{
		points[i] *= uInverseProjMatrix;// * points[i];
		//points[i] /= points[i] .w;
	}
	pointLightsIndex[id * 4 + offset] = -999999;
	// Compute frustums plan
	for(int i = 0; i < 4; ++i)
	{
		vec3 normal = vec3(ivec3(cross(points[i % 4].xyz, points[(i + 1) % 4].xyz)));//normalize(ivec3(cross(points[i % 4].xyz, points[(i + 1) % 4].xyz))); // TODO : refacto function
		float d = dot(normal, points[i].xyz);
		normal = normalize(normal);
		frustumPlans[i] = vec4(normal, d);
	}
	
	
	for(id = 0; id < 6; ++id)
	{
		pointLightsIndex[id * 4 + offset] = int(frustumPlans[id].x * 100);
		pointLightsIndex[id * 4 + offset + 1] = int(frustumPlans[id].y * 100);
		pointLightsIndex[id * 4 + offset + 2] = int(frustumPlans[id].z * 100);
		pointLightsIndex[id * 4 + offset + 3] = int(frustumPlans[id].w * 100);
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
		vec3 pointLightCoords = (uViewMatrix * pointLights[i].position).xyz;

		// Check depth
		//pointLightsIndex[threadIndex * 5 + i] = int(pointLights[lightIndex].radiusAttenuation);		
		//pointLightsIndex[threadIndex * 5 + i + 1] = int(pointLightCoords.x);
		//pointLightsIndex[threadIndex * 5 + i + 2] = int(pointLightCoords.y);
		//pointLightsIndex[threadIndex * 5 + i + 3] = int(pointLightCoords.z);
		//pointLightsIndex[threadIndex * 5 + i + 4] = int(-1);

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
		
		frustumPlans[4] = vec4(0,0,-1,-depthMin) * uViewMatrix;
		frustumPlans[4] /= length(frustumPlans[4].xyz);

		frustumPlans[5] = vec4(0,0,1,depthMax) *  uViewMatrix;
		frustumPlans[5] /= length(frustumPlans[5].xyz);

		initFrustum(pixelGlobalPosition, pixelLocalPosition, tilePosition, numTiles, tileIndex);
	}
	barrier();

	lightCulling(threadIndex);
	
	barrier();
	if(threadIndex == 0)
	{
		//pointLightsIndex[30] = -1;
		int offset = int(tilePosition.x + tilePosition.y * numTiles.x) * MAX_LIGHTS;
		int i = 0;
		//for(i = 0; i < countTilePointLight; ++i)
		//{
		//	pointLightsIndex[i + offset] = tilePointLightsIndex[i];
		//}
		//if(countTilePointLight <= MAX_LIGHTS)
		//{
		//	int indexMinus1 = i + offset;
		//	if(countTilePointLight != 0)
		//		indexMinus1 += 1; 
		//	pointLightsIndex[indexMinus1] = -1; // To mark limite data.
		//}
	}

}