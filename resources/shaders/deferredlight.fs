#version 430

in vec2 vsTexcoord;

layout(location = 0) out vec4 outColor;

layout(binding=0) uniform sampler2D uSamplerColor;
layout(binding=1) uniform sampler2D uSamplerDepth;
layout(binding=2) uniform sampler2D uSamplerNormal;

@ubo.inc
@utils.inc //decodeNormal
@lights.inc

vec3 DepthToPos(float depth, vec2 texCoord)
{
	vec4 ndcspace = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 temp = inverse(PerFrame.view) * inverse(PerFrame.proj) * ndcspace;
	return temp.xyz / temp.w;
}

float Attenuation(float distance, float range, float constant, float lin, float quad)
{
	float atten = 1.0 / (constant + lin * distance + quad * distance * distance);
	return step(distance, range) * clamp(atten, 0.0, 1.0);
}

vec4 doLight(Light light, vec3 norm, vec3 worldPosition, vec4 diff)
{
	vec3 positionToLight = light.position.xyz - worldPosition;
	vec3 posToLightDir   = normalize(positionToLight);

	float cosAngIncidence = dot(posToLightDir, norm);
	cosAngIncidence       = clamp(cosAngIncidence, 0.0, 1.0);

	float a = Attenuation(length(positionToLight), light.range, light.constantAttenuation, light.linearAttenuation, light.quadraticAttenuation);
	
	vec4 result = cosAngIncidence * diff * light.diffuse; // Diffuse
	return result * vec4(a,a,a,1); // Attenuation
}

void main()
{
	vec4 diff = texture(uSamplerColor, vsTexcoord);
	vec3 norm = decodeNormal(texture(uSamplerNormal, vsTexcoord));
	float depth = texture(uSamplerDepth, vsTexcoord).r;

	//outColor = diff;
	//return;

	vec3 worldPosition = DepthToPos(depth, vsTexcoord); 

	outColor = vec4(0,0,0,1);

	if(depth != 1.0) 
	{
		for(int i = 0; i < Lights.num; ++i)
		{
			Light light = Lights.lights[i];
			outColor += doLight(light, norm, worldPosition, diff);
		}
	} 
	else 
	{
		outColor = diff;
	}
}