#version 430 core

in vec2 vsTexcoord;

layout(location = 0) out vec4 outColor;

layout(binding=0) uniform sampler2D uSamplerColor;

void main()
{
	vec4 diff = texture(uSamplerColor, vsTexcoord);
	outColor = diff;
}