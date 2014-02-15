#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec3 vsPosition;
out vec2 vsTexcoord;
out vec3 vsNormal;
out vec3 vsTangent;
out vec3 vsBitangent;

@ubo.inc // PerFrame

void main()
{
	vsTexcoord = vec2(texcoord.x, 1.0 - texcoord.y); // Note; flipped Y
	vsNormal = normal;
	vsTangent = tangent;
	vsBitangent = bitangent;

	vec4 positionWorld = PerDraw.modelMatrix * vec4(position, 1.0);
	vsPosition = positionWorld.xyz;

	gl_Position = PerFrame.proj * PerFrame.view  * positionWorld;
}