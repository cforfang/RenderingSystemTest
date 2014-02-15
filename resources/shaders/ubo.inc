layout(std140, binding = 0) uniform PerFrameUBO
{  
   mat4 proj;
   mat4 view;
   vec4 cameraPosition;
   float nearPlane; float farPlane; double time;
   uint flags;
} PerFrame;

layout(std140, binding = 1) uniform PerDrawUBO
{  
   mat4 modelMatrix;
} PerDraw;