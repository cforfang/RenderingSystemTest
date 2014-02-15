vec3 decodeNormal(vec4 normal)
{
	return vec3(normal * 2.0 - vec4(vec3(1.0), 0.0));
}

vec4 encodeNormal(vec3 normal)
{
	return vec4(normal * 0.5 + vec3(0.5), 1.0);
}
