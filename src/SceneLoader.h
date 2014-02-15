#pragma once

#include <string>
#include <vector>

#include <stdint.h>
#include "glm/glm.hpp"

namespace SceneLoader
{
	struct MeshInfo
	{
		uint32_t materialIndex;

		glm::vec3 position;
		float radius;

		uint32_t numVertices;
		uint32_t vertexDataSize;
		uint32_t vertexDataOffset;

		uint32_t numIndices;
		uint32_t indexDataSize;
		uint32_t indexDataOffset;
	};

	struct MaterialInfo
	{
		std::string name;
		std::string diffuseTexture;
		std::string specularTexture;
		std::string normalTexture;
		std::string heightTexture;

		glm::vec4 diffuse;
		glm::vec4 ambient;
		glm::vec4 specular;
		glm::vec4 emissive;

		float shininess;
	};

	struct SceneInfo
	{
		std::string meshDataFile;
		std::vector<MeshInfo> meshes;
		std::vector<MaterialInfo> materials;
	};

	extern bool LoadScene(const std::string& luaFile, SceneInfo& outSceneInfo);
}