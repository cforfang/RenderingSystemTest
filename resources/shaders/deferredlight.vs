#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

@ubo.inc

out vec2 vsTexcoord;

void main()
{
	vsTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);
	gl_Position = vec4(position, 1.0);
}