#pragma once

#include "graphics/Handles.h"
#include "glm/glm.hpp"

struct PerFrameUBO
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec4 cameraPosition;
	float nearPlane; float farPlane; double time;

	enum Flags
	{
		NormalMapping = 1 << 0,
		ParallaxMapping = 1 << 1,
		SSAOState = 1 << 2,
	};
	glm::uint flags = 0;
};

struct DrawUBO
{
	glm::mat4 modelMatrix;
};

struct MaterialUBO
{
	enum Flags
	{
		HasNormalMap = 1 << 0,
		HasHeightMap = 1 << 1,
	};
	glm::uint flags = 0;
};

struct Mesh
{
	Graphics::BufferHandle vertexBuffer;
	Graphics::BufferHandle indexBuffer;
	uint32_t numElements;
	float radius;
};