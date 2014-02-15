#version 430 core

in vec3 vsPosition;
in vec2 vsTexcoord;
in vec3 vsNormal;
in vec3 vsTangent;
in vec3 vsBitangent;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

layout(binding=0) uniform sampler2D uSamplerDiffuse;
layout(binding=1) uniform sampler2D uSamplerNormal;
layout(binding=2) uniform sampler2D uSamplerHeight;
layout(binding=3) uniform sampler2D uSamplerSpecular;

@utils.inc // encodeNormal
@material.inc
@ubo.inc

mat3 ComputeTBN()
{
	vec3 n = vsNormal;
	vec3 t = normalize(vsTangent);
	vec3 b = normalize(vsBitangent);
	return mat3(t,b,n);
}

// For parallax mapping
const float heightScale = 0.025; 

// For normal-generation from height-map
const float normalSmooth = 1.5; // steeper < 1.0 < smoother

void main()
{
	bool normalMappingEnabled   = (PerFrame.flags & 0x1) == 0x1;
	bool parallaxMappingEnabled = (PerFrame.flags & 0x2) == 0x2;
	bool materialHasNormalMap = (Material.flags & 0x1) == 0x1;
	bool materialHasHeightMap = (Material.flags & 0x2) == 0x2;

	vec2 texCoord = vsTexcoord;

	if(materialHasHeightMap && parallaxMappingEnabled)
	{
		// Calculate new texcoord using parallax mapping

		vec3 cameraPos = PerFrame.cameraPosition.xyz;
		vec3 fragPos   = vsPosition;

		// Calculate tangent-space directions towards camera
		vec3 viewdir = -normalize(fragPos - cameraPos);
		viewdir = normalize( transpose(ComputeTBN()) * viewdir ); // transpose = inverse here

		float fBumpScale  = heightScale;
		float bias = (fBumpScale / 2.0f);
		float height = texture(uSamplerHeight, vsTexcoord).r;
		vec2 halfOffset = normalize(viewdir).xy * (height * fBumpScale - bias);

		for(int i = 0; i < 2; ++i)
		{
			height = (height + texture(uSamplerHeight, vsTexcoord + halfOffset).r) * 0.5;
			halfOffset = normalize(viewdir).xy * (height * fBumpScale - bias);
		}

		texCoord = vsTexcoord + halfOffset;
	}

	vec4 diff = texture(uSamplerDiffuse, texCoord);
	outColor = diff;

	// Sponza-spesific: mask is in diffuse alpha
	if(diff.a < 0.5) 
		discard;

	vec3 normal = vsNormal;

	if (materialHasNormalMap && normalMappingEnabled)
	{
		// Extract normal from normal map

		vec3 normalTex = texture(uSamplerNormal, texCoord).rgb;
		normalTex = (2.0 * normalTex) - vec3(1.0);
		normal = normalize(ComputeTBN() * normalTex);
	}
	else if (materialHasHeightMap && normalMappingEnabled)
	{
		// Calculate new normal from height map using a Sobel filter

		vec2 vPixelSize = vec2(1, 1)/textureSize(uSamplerHeight, 0);

		// Compute the necessary offsets:
		vec2 o00 = texCoord + vec2( -vPixelSize.x, -vPixelSize.y );
		vec2 o10 = texCoord + vec2(          0.0f, -vPixelSize.y );
		vec2 o20 = texCoord + vec2(  vPixelSize.x, -vPixelSize.y );
 
		vec2 o01 = texCoord + vec2( -vPixelSize.x, 0.0f          );
		vec2 o21 = texCoord + vec2(  vPixelSize.x, 0.0f          );
 
		vec2 o02 = texCoord + vec2( -vPixelSize.x,  vPixelSize.y );
		vec2 o12 = texCoord + vec2(          0.0f,  vPixelSize.y );
		vec2 o22 = texCoord + vec2(  vPixelSize.x,  vPixelSize.y );
 
		// Use of the sobel filter requires the eight samples
		// surrounding the current pixel:
		float h00 = texture(uSamplerHeight, o00 ).r;
		float h10 = texture(uSamplerHeight, o10 ).r;
		float h20 = texture(uSamplerHeight, o20 ).r;
 
		float h01 = texture(uSamplerHeight, o01 ).r;
		float h21 = texture(uSamplerHeight, o21 ).r;
 
		float h02 = texture(uSamplerHeight, o02 ).r;
		float h12 = texture(uSamplerHeight, o12 ).r;
		float h22 = texture(uSamplerHeight, o22 ).r;
 
		// Evaluate the Sobel filters
		float Gx = h00 - h20 + 2.0f * h01 - 2.0f * h21 + h02 - h22;
		float Gy = h00 + 2.0f * h10 + h20 - h02 - 2.0f * h12 - h22;
 
		// Generate the missing Z
		float Gz = normalSmooth * sqrt( 1.0f - Gx * Gx - Gy * Gy );
 
		// Make sure the returned normal is of unit length
 		normal = normalize( ComputeTBN() *  vec3( 2.0f * Gx, 2.0f * Gy, Gz ) );
	}
	
	outNormal = encodeNormal(normal);

	// Debug
	//outColor = outNormal;
	//outColor = vec4(normal, 1);
}