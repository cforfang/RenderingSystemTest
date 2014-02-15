struct Light
{
  vec4 position;
  vec4 diffuse;
  vec4 specular;
  float constantAttenuation; float linearAttenuation; float quadraticAttenuation; float range;
};

layout(std140, binding = 2) uniform LightUBO
{  
  uint num;
  Light lights[100];
} Lights;