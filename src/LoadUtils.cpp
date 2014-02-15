#include "LoadUtils.h"

#include <unordered_map>
#include <fstream>
#include <thread>
#include <atomic>

#include "graphics/RenderingSystem.h"

#include "Renderable.h"
#include "Material.h"
#include "SceneLoader.h"
#include "TextureLoader.h"
#include "UBOsAndMesh.h"

namespace
{
	// Scales positions of loaded meshes. 
	// These are appropriate for Crytek Sponza, so that 1 world-space unit ~= 1m.
	const float LOAD_POSITION_SCALE = 0.01f;
	const float LOAD_DEFAULT_SCALE = 0.01f;

	Material CreateMaterial(const SceneLoader::MaterialInfo& materialInfo, Graphics::RenderingSystem& renderingSystem, TextureLoader& textureLoader)
	{
		MaterialUBO materialBufferUBO;

		Graphics::BufferHandle materialBufferHandle = Graphics::BufferHandle::Invalid();
		Graphics::Texture2DHandle diffTexHandle     = Graphics::Texture2DHandle::Invalid();
		Graphics::Texture2DHandle normalTexHandle   = Graphics::Texture2DHandle::Invalid();
		Graphics::Texture2DHandle heightTexHandle   = Graphics::Texture2DHandle::Invalid();

		if (materialInfo.diffuseTexture != "")
		{
			// Load as SRGB
			diffTexHandle = renderingSystem.CreateTexture2D();
			textureLoader.Schedule({ diffTexHandle, materialInfo.diffuseTexture, Graphics::TextureType::SRGBA8 });
		}

		if (materialInfo.normalTexture != "")
		{
			// Load as RGB
			normalTexHandle = renderingSystem.CreateTexture2D();
			materialBufferUBO.flags |= MaterialUBO::HasNormalMap;
			textureLoader.Schedule({ normalTexHandle, materialInfo.normalTexture, Graphics::TextureType::RGBA8 });
		}

		if (materialInfo.heightTexture != "")
		{
			// Load as R8
			heightTexHandle = renderingSystem.CreateTexture2D();
			materialBufferUBO.flags |= MaterialUBO::HasHeightMap;
			textureLoader.Schedule({ heightTexHandle, materialInfo.heightTexture, Graphics::TextureType::R8 });
		}

		// Create and upload material-UBO
		materialBufferHandle = renderingSystem.CreateBuffer();
		renderingSystem.UpdateBuffer(materialBufferHandle, &materialBufferUBO, sizeof(materialBufferUBO), Graphics::BufferType::STATIC);

		return Material{ diffTexHandle, normalTexHandle, heightTexHandle, materialBufferHandle };
	}
}

bool GetRenderables(const std::string& dataPrefix, TextureLoader& textureLoader, Graphics::RenderingSystem& renderingSystem, std::vector<Renderable>& outRenderables)
{
	// Load scene
	SceneLoader::SceneInfo sceneInfo;

	const std::string sceneFile = dataPrefix + "scene.lua";
	printf("Processing %s...\n", sceneFile.c_str());

	bool success = SceneLoader::LoadScene(sceneFile, sceneInfo);

	if (!success)
	{
		printf("Error loading scene\n");
		return false;
	}

	// Load materials
	std::unordered_map<size_t, Material> loadedMaterials;
	printf("Loading materials...\n");

	for (size_t index = 0; index < sceneInfo.materials.size(); ++index)
	{
		auto& materialInfo = sceneInfo.materials[index];
		loadedMaterials.emplace(index, CreateMaterial(materialInfo, renderingSystem, textureLoader));		
	}

	// Load datafile
	std::ifstream meshDataStream(dataPrefix + sceneInfo.meshDataFile, std::ios::in | std::ios::binary | std::ios::ate);
	if (!meshDataStream.is_open())
	{
		printf("Failed to load %s.\n", sceneInfo.meshDataFile.c_str());
		return 0;
	}

	renderingSystem.SetWindowTitle("Loading...");

	// Do data-loading in separate thread (can take a while for big scenes -- e.g. San Miguel = ~1.2GB)
	std::atomic<bool> done{false};
	std::vector<char> meshDataVector;

	auto thread = std::thread([&]()
	{
		std::ifstream::pos_type fileSize = meshDataStream.tellg();
		printf("Loading mesh-data from %s (%.2f MB)...\n", (dataPrefix + sceneInfo.meshDataFile).c_str(), fileSize / 1024.0f / 1024.0f);
		
		meshDataStream.seekg(0, std::ios::beg);

		meshDataVector = std::vector<char>((
			std::istreambuf_iterator<char>(meshDataStream)),
			(std::istreambuf_iterator<char>()));

		done = true;
	});

	renderingSystem.SetWindowTitle("Loading...");

	// Poll the window while loading
	while (!done)
	{
		renderingSystem.PollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	renderingSystem.SetWindowTitle("");

	// Done loading, so join the thread and close file
	thread.join();
	meshDataStream.close();

	// Load meshes
	const size_t numToLoad = sceneInfo.meshes.size();
	printf("Loading %u meshes...\n", numToLoad);

	for (size_t index = 0; index < numToLoad; ++index)
	{
		// Get meshinfo from scene
		auto& meshInfo = sceneInfo.meshes[index];

		// Find material
		auto it = loadedMaterials.find(meshInfo.materialIndex);
		if (it == loadedMaterials.end())
		{
			// TODO Default material
			fprintf(stderr, "Material-index %u referred by mesh wasn't found. Skipped mesh.\n", meshInfo.materialIndex);
			continue;
		}

		const Material& material = it->second;

		Mesh newMesh;
		newMesh.radius = meshInfo.radius;

		// Create and upload vertex-buffer
		void* verticesDataAddress = meshDataVector.data() + meshInfo.vertexDataOffset;
		uint32_t verticesDataSize = meshInfo.vertexDataSize;
		assert(meshDataVector.size() >= meshInfo.vertexDataOffset + verticesDataSize);

		newMesh.vertexBuffer = renderingSystem.CreateBuffer();
		renderingSystem.UpdateBuffer(newMesh.vertexBuffer, verticesDataAddress, verticesDataSize, Graphics::BufferType::STATIC);

		// Overriden if mesh has index-buffer
		newMesh.numElements = meshInfo.numVertices;

		// Create and upload index-buffer
		if (meshInfo.numIndices)
		{
			void* indicesDataAddress = meshDataVector.data() + meshInfo.indexDataOffset;
			uint32_t indicesDataSize = meshInfo.indexDataSize;
			assert(meshDataVector.size() >= meshInfo.indexDataOffset + indicesDataSize);

			newMesh.indexBuffer = renderingSystem.CreateBuffer();
			newMesh.numElements = meshInfo.numIndices;

			renderingSystem.UpdateBuffer(newMesh.indexBuffer, indicesDataAddress, indicesDataSize, Graphics::BufferType::STATIC);
		}
		else
		{
			newMesh.indexBuffer = Graphics::BufferHandle::Invalid();
		}

		// Create a renderable for this mesh
		Renderable renderable(newMesh, material);
		renderable.SetPosition(meshInfo.position * LOAD_POSITION_SCALE);
		renderable.SetScale(LOAD_DEFAULT_SCALE);

		outRenderables.push_back(renderable);

		// Poll at regular intervals
		if (index % 100 == 0)
		{
			printf("Processing meshes: %.2f%%...\n", index / (float) numToLoad * 100.0f);
			renderingSystem.PollEvents();
		}
	}

	return true;
}