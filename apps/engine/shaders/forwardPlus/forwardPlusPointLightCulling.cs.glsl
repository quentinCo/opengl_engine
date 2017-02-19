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
	vec4 debugOutput[];
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
shared float depthMinFloat;
shared float depthMaxFloat;
//shared float nearPlan;

vec4 clipSpaceToViewSpace(vec4 point)
{
	vec4 viewPoint = uInverseProjMatrix * point;
	viewPoint = viewPoint / viewPoint.w;
	return viewPoint;
}

vec4 screenSpaceToViewSpace(vec4 point)
{
	vec4 clipPoint = vec4(point.xy / uWindowDim, point.z, point.w); // normalize the point xy
	clipPoint.xy = vec2((2 * clipPoint.x - 1), (2 * (1 - clipPoint.y) - 1));
	clipPoint = clipSpaceToViewSpace(clipPoint);
	return clipPoint;
}

vec4 computeFrustumPlan(vec4 point1, vec4 point2)
{
	// Origin is on 0,0,0
	vec3 normal = normalize(cross(point1.xyz, point2.xyz));
	float distanceToOrigin = dot(normal, point1.xyz);
	return vec4(normal, distanceToOrigin);
}

void initFrustum(vec2 pixelGlobalPosition, vec2 pixelLocalPosition, vec2 tilePosition, vec2 numTiles, int tileIndex)
{
	vec4 points[4];
	// Screen space
	points[0] = vec4(pixelGlobalPosition, -1, 1);
	points[1] = vec4((tilePosition.x + 1) * TILE_SIZE, pixelGlobalPosition.y, -1, 1);
	points[2] = vec4((tilePosition.x + 1) * TILE_SIZE, (tilePosition.y + 1) * TILE_SIZE, -1, 1);
	points[3] = vec4(pixelGlobalPosition.x, (tilePosition.y + 1) * TILE_SIZE, -1, 1);

	// Clip space
	for(int i = 0; i < 4; ++i)
	{
		points[i] = screenSpaceToViewSpace(points[i]);
	}
	
	// Compute frustums plan
	for(int i = 0; i < 4; ++i)
	{
		frustumPlans[i] = computeFrustumPlan(points[i], points[(i + 1) % 4]);
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

		vec3 pointLightCoords = (uViewMatrix * vec4(pointLights[lightIndex].position)).xyz;
		
		int offset = int(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x) * 4 ; 
		//debugOutput[offset + lightIndex] = vec4(pointLightCoords,  pointLights[lightIndex].radiusAttenuation);

		// Check depth
		//debugOutput[offset + lightIndex] = vec4(frustumPlans[4].w,  frustumPlans[5].w, 0 , 0);
		//debugOutput[offset + lightIndex] = vec4(depthMinFloat,  depthMaxFloat, 0 , 0);
		// Watch in -z
		if(pointLightCoords.z - pointLights[lightIndex].radiusAttenuation > depthMinFloat ||
		 pointLightCoords.z + pointLights[lightIndex].radiusAttenuation < depthMaxFloat)
		{
			continue;	
		}
		debugOutput[offset + lightIndex] = vec4(threadIndex, pointLightCoords.z, depthMinFloat, depthMaxFloat);

		// Check distance from pointLight to frustum
		bool intersect = false;
		for(int j = 0; j < 4; ++j)
		{
			// distance = dot(v, n) / length(n) -> length(n) = 1;
			//if(dot(frustumPlans[j].xyz, pointLightCoords.xyz) - frustumPlans[j].w < -pointLights[lightIndex].radiusAttenuation)
			if(dot(frustumPlans[j].xyz, pointLightCoords.xyz) - frustumPlans[j].w >= -pointLights[lightIndex].radiusAttenuation)
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
	int threadIndex = int(pixelLocalPosition.x + pixelLocalPosition.y * TILE_SIZE );
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
		//depthMinFloat = (0.5 * uProjMatrix[3][2]) / (uintBitsToFloat(depthMinInt) + 0.5 * uProjMatrix[2][2] - 0.5);
		//depthMaxFloat = (0.5 * uProjMatrix[3][2]) / (uintBitsToFloat(depthMaxInt) + 0.5 * uProjMatrix[2][2] - 0.5);
		
		//frustumPlans[4] = uViewMatrix * vec4(0,0,-1,depthMinFloat);
		//frustumPlans[4] /= length(frustumPlans[4].xyz);
		//frustumPlans[5] = uViewMatrix * vec4(0,0,1,depthMaxFloat);
		//frustumPlans[5] /= length(frustumPlans[5].xyz);

		initFrustum(pixelGlobalPosition, pixelLocalPosition, tilePosition, numTiles, tileIndex);
		depthMinFloat = (2 * uintBitsToFloat(depthMinInt) - 1); // Space [0 ; 1] to [-1 ; 1]
		depthMaxFloat = (2 * uintBitsToFloat(depthMaxInt) - 1);
		depthMinFloat = clipSpaceToViewSpace(vec4(0, 0, depthMinFloat, 1)).z;
		depthMaxFloat = clipSpaceToViewSpace(vec4(0, 0, depthMaxFloat, 1)).z;
		//nearPlan = clipSpaceToViewSpace(vec4(0, 0, 0, 1)).z;
		
		frustumPlans[4] = vec4(0, 0, -1, depthMinFloat);
		frustumPlans[5] = vec4(0, 0, 1, depthMaxFloat);	
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
			//debugOutput[i + offset] = tilePointLightsIndex[i];
		}
		if(countTilePointLight <= MAX_LIGHTS)
		{
			int indexMinus1 = i + offset;
			pointLightsIndex[indexMinus1] = -1; // To mark limite data.
			//debugOutput[indexMinus1] = -1;
		}
	}

}