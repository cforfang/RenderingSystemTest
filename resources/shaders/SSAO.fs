#version 430

in vec2 vsTexcoord;

layout(location = 0) out vec4 outColor;

layout(binding=0) uniform sampler2D uSamplerColor;
layout(binding=1) uniform sampler2D uSamplerDepth;
layout(binding=2) uniform sampler2D uSamplerNormal;

@ubo.inc

// 0 = off
#define SSAO_STRENGTH 1.50

vec3 depthToPos(float depth, vec2 texCoord)
{
	vec4 ndcspace = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 temp = inverse(PerFrame.view) * inverse(PerFrame.proj) * ndcspace;
	return temp.xyz / temp.w;
}

float rand(vec2 co)
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453) * 2 - 1;
}

const int sample_count = 16;

const vec2 poisson16[] = vec2[](
	vec2( -0.94201624,  -0.39906216 ),
	vec2(  0.94558609,  -0.76890725 ),
	vec2( -0.094184101, -0.92938870 ),
	vec2(  0.34495938,   0.29387760 ),
	vec2( -0.91588581,   0.45771432 ),
	vec2( -0.81544232,  -0.87912464 ),
	vec2( -0.38277543,   0.27676845 ),
	vec2(  0.97484398,   0.75648379 ),
	vec2(  0.44323325,  -0.97511554 ),
	vec2(  0.53742981,  -0.47373420 ),
	vec2( -0.26496911,  -0.41893023 ),
	vec2(  0.79197514,   0.19090188 ),
	vec2( -0.24188840,   0.99706507 ),
	vec2( -0.81409955,   0.91437590 ),
	vec2(  0.19984126,   0.78641367 ),
	vec2(  0.14383161,  -0.14100790 )
);

mat2 randomRotation( const vec2 p )
{
	float r = rand(p);
	float sinr = sin(r);
	float cosr = cos(r);
	return mat2(cosr, sinr, -sinr, cosr);
}

@utils.inc //decodeNormal

void main()
{
	vec4  color  = texture(uSamplerColor, vsTexcoord);
	vec3  normal = decodeNormal(texture(uSamplerNormal, vsTexcoord));
	float depth  = texture(uSamplerDepth, vsTexcoord).x;

	if(depth == 1.0)
	{
		outColor = vec4(color.rgb, 1);
		return;
	}
	
	vec3 worldPosition = depthToPos(depth, vsTexcoord); 
	vec3 viewPos = vec3(PerFrame.view * vec4(worldPosition, 1));

	// Sample-radius
	vec2 texRad = vec2(100.0, 100.0);
	texRad /= viewPos.z; // Smaller with distance
	vec2 radius = texRad / textureSize(uSamplerColor, 0);

	float distanceThreshold = 0.25;
	float ambientOcclusion = 0;

	for (int i = 0; i < sample_count; ++i)
	{
		vec2  sampleTexCoord = vsTexcoord + (randomRotation(vsTexcoord) * poisson16[i] * radius);
		float sampleDepth    = texture(uSamplerDepth, sampleTexCoord).x;

		vec3  samplePos      = depthToPos(sampleDepth, sampleTexCoord); 
		float sampleDistance = distance(worldPosition, samplePos);
		vec3  sampleDir      = normalize(samplePos - worldPosition);

		// Angle between the normal and direction to sampled location
		float NdotS = max(dot(normal.xyz, sampleDir), 0);

		// Fade out occlusion with distance
		float fade = 1.0 - smoothstep(distanceThreshold, distanceThreshold * 2, sampleDistance);

		// Discard samples outside screen
		int onScreen = 1 - int(sampleTexCoord.x > 1 || sampleTexCoord.x < 0 || sampleTexCoord.y > 1 || sampleTexCoord.y < 0);

		ambientOcclusion += (fade * NdotS) * onScreen;
	}

	float result = (1.0 - (ambientOcclusion/sample_count));
	result = pow(result, SSAO_STRENGTH);

	if((PerFrame.flags & 0x4) == 0x4)
		outColor = vec4(vec3(result), 1); // Occlusion only
	else
		outColor = vec4(color.rgb * result, 1); // Color
}