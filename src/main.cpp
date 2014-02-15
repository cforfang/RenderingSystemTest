#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "graphics/RenderingSystem.h"

#include "Constants.h"
#include "UBOsAndMesh.h"

#include "MeshUtils.h"
#include "LoadUtils.h"

#include "Renderable.h"
#include "Material.h"

#include "FrustumCuller.h"
#include "PostProcess.h"
#include "TextureLoader.h"
#include "LightManager.h"

namespace
{
	const std::string dataFolder = "data/";

	void DrawRenderables(Graphics::RenderingSystem& renderingSystem, std::vector<Renderable>& renderables, const std::vector<bool>& isCulled, Graphics::BufferHandle& perDrawUBOHandle)
	{
		DrawUBO perDrawUBO;

		for (size_t i = 0; i < renderables.size(); ++i)
		{
			if (isCulled[i])
				continue;

			Renderable& renderable = renderables[i];
			renderable.GetMaterial().Bind(renderingSystem);

			perDrawUBO.modelMatrix = renderable.GetModelMatrix();
			renderingSystem.UpdateBuffer(perDrawUBOHandle, &perDrawUBO, sizeof(perDrawUBO), Graphics::BufferType::DYNAMIC);

			renderingSystem.Draw(renderable.GetMesh().vertexBuffer, renderable.GetMesh().indexBuffer, renderable.GetMesh().numElements);
		}
	}
}

int main(int argc, char* argv[])
{
	Graphics::WindowConfig wc;
	wc.width = 1280;
	wc.height = 720;
	wc.resizable = false;
	wc.mode = Graphics::WindowMode::WINDOW;

	Graphics::ContextConfig cc;
	cc.msaaSamples = 4;
	cc.debugContext = true;
	cc.glewExperimental = true;
	cc.coreProfileContext = true;
#ifdef _DEBUG
	cc.synchronousDebugOutput = true;
#else
	cc.synchronousDebugOutput = false;
#endif

	Graphics::RenderingSystem renderingSystem;

	if (!renderingSystem.Init(wc, cc))
	{
		fprintf(stderr, "Error while initializing renderingsystem.\n");
		return 1;
	}

	auto deferredShader = renderingSystem.CreateShaderProgram(
		Graphics::ShaderInfo::VSFS("shaders/deferred.vs", "shaders/deferred.fs", "shaders/")
		);

	auto deferredLightShader = renderingSystem.CreateShaderProgram(
		Graphics::ShaderInfo::VSFS("shaders/deferredlight.vs", "shaders/deferredlight.fs", "shaders/")
		);

	auto copyShader = renderingSystem.CreateShaderProgram(
		Graphics::ShaderInfo::VSFS("shaders/copy.vs", "shaders/copy.fs", "shaders/")
		);

	// Full-screen quad
	auto quadVertexBuffer = renderingSystem.CreateBuffer();
	renderingSystem.UpdateBuffer(quadVertexBuffer, MeshUtils::quadVertices.data(), MeshUtils::quadVertices.size() * sizeof(float), Graphics::BufferType::STATIC);

	// Postprocessing
	PostProcess_SSAO ssaoPP;
	ssaoPP.Init(renderingSystem, quadVertexBuffer);

	// Rendertargets
	auto gBufferRT = renderingSystem.CreateRenderTarget(Graphics::RenderTargetOptions::SRGB8DepthRGB8(wc.width, wc.height));
	auto tempRT = renderingSystem.CreateRenderTarget(Graphics::RenderTargetOptions::SRGB8Depth(wc.width, wc.height));

	// For per-frame shader data
	PerFrameUBO perFrameUBO;
	perFrameUBO.nearPlane = 0.1f;
	perFrameUBO.farPlane = 100.0f;
	perFrameUBO.proj = glm::perspective(45.0f, wc.width / (float)wc.height, perFrameUBO.nearPlane, perFrameUBO.farPlane);
	perFrameUBO.flags = PerFrameUBO::Flags::NormalMapping | PerFrameUBO::Flags::ParallaxMapping;

	auto perFrameUBOHandle = renderingSystem.CreateBuffer();
	renderingSystem.UpdateBuffer(perFrameUBOHandle, NULL, sizeof(perFrameUBO), Graphics::BufferType::DYNAMIC);
	renderingSystem.BindUniformBuffer(Constants::PER_FRAME_UBO_BINDING_INDEX, perFrameUBOHandle);

	// Used for per-drawcall shader data 
	auto perDrawUBOHandle = renderingSystem.CreateBuffer();
	renderingSystem.UpdateBuffer(perDrawUBOHandle, NULL, sizeof(DrawUBO), Graphics::BufferType::DYNAMIC);
	renderingSystem.BindUniformBuffer(Constants::PER_DRAW_UBO_BINDING_INDEX, perDrawUBOHandle);

	// Animates the lights and updates uniform buffer with lightdata
	LightManager<10> lightManager;
	lightManager.Init(renderingSystem);

	// To avoid having to load all textures at startup
	TextureLoader textureLoader;

	// Load the scene.
	// This can take a while for big scenes, so it'll poll the window to keep it responsive.
	std::vector<Renderable> renderables;
	if (!GetRenderables(dataFolder, textureLoader, renderingSystem, renderables /*out*/))
	{
		return 0;
	}

	// Used for culling
	std::vector<bool> isCulled;
	FrustumCuller<Renderable> frustumCuller;

	glm::vec3 cameraPosition(0, 2, 0);
	glm::quat cameraOrientation;

	uint8_t ssaoState = 1;
	const std::array<const char*, 3> ssaoText = {{ "Off", "On", "Occlusion only" }};

	bool parallaxMappingEnabled = true;
	bool normalMappingEnabled = true;

	int frames = 0;
	double timeAccum = 0.0;
	double lastTime = 0.0;

	while (!renderingSystem.CloseRequested())
	{
		double time = renderingSystem.GetTime();
		double dt = time - lastTime;
		lastTime = time;
		timeAccum += dt;

		// Sort renderables by material (could only be done once since it's not dynamic)
		std::sort(renderables.begin(), renderables.end(), [](const Renderable& a, const Renderable& b)
		{
			return a.GetMaterial() < b.GetMaterial();
		});

		// Update movement
		glm::vec3 movement(0, 0, 0);
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::W)) movement.z -= 1.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::S)) movement.z += 1.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::A)) movement.x -= 1.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::D)) movement.x += 1.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::Q)) movement.y -= 1.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::E)) movement.y += 1.0f;

		float movementScale = 3.0f;
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::SHIFT))
			movementScale *= 4.0f;

		if (glm::length(movement) > 0)
			movement = glm::normalize(movement) * movementScale * float(dt);

		cameraPosition += movement * cameraOrientation;

		// Mouse look
		static glm::vec2 lastCursor = renderingSystem.GetCursorPosition();
		auto cursor = renderingSystem.GetCursorPosition();
		glm::vec2 diff = lastCursor - cursor;
		lastCursor = cursor;

		if (renderingSystem.IsMouseButtonDown(Graphics::RenderingSystem::MouseButton::Right))
		{
			renderingSystem.SetCursorEnabled(false);
			diff *= 0.1f;
			cameraOrientation = cameraOrientation * glm::rotate(glm::quat(), -diff.x, glm::vec3(0, 1, 0)); // Yaw
			cameraOrientation = glm::rotate(glm::quat(), -diff.y, glm::vec3(1, 0, 0)) * cameraOrientation; // Pitch
		}
		else renderingSystem.SetCursorEnabled(true);

		// Update per-frame UBO
		perFrameUBO.view = glm::toMat4(cameraOrientation) * glm::translate(glm::mat4(1.0f), -cameraPosition);
		perFrameUBO.cameraPosition = glm::vec4(cameraPosition, 1.0);
		perFrameUBO.time = time;
		perFrameUBO.flags = 0;

		if (parallaxMappingEnabled)
			perFrameUBO.flags |= PerFrameUBO::Flags::ParallaxMapping;

		if(normalMappingEnabled)
			perFrameUBO.flags |= PerFrameUBO::Flags::NormalMapping;

		if (ssaoState == 2)
			perFrameUBO.flags |= PerFrameUBO::Flags::SSAOState;

		renderingSystem.UpdateBuffer(perFrameUBOHandle, &perFrameUBO, sizeof(perFrameUBO), Graphics::BufferType::DYNAMIC);

		// Update lights
		lightManager.Update(time, cameraPosition);

		// Do frustum-culling
		frustumCuller.Cull(renderables, isCulled, perFrameUBO.proj * perFrameUBO.view);
#if 1
		// Draw to G-buffer
		{
			// Bind G-buffer
			renderingSystem.BindRenderTarget(gBufferRT);

			// Clear it
			renderingSystem.ClearScreen(Graphics::ClearState::AllBuffers());

			// Prepare to fill it
			renderingSystem.UseShaderProgram(deferredShader);
			renderingSystem.BindUniformBuffer(Constants::PER_FRAME_UBO_BINDING_INDEX, perFrameUBOHandle);
			renderingSystem.BindUniformBuffer(Constants::PER_DRAW_UBO_BINDING_INDEX, perDrawUBOHandle);

			// Draw objects
			DrawRenderables(renderingSystem, renderables, isCulled, perDrawUBOHandle);
		}

		// Do lighting to temporary rendertarget (all lights in one pass -- extremly wasteful; 
		// would really want to cull and then draw individual bounding spheres/quads or similar).
		{
			// Prepare G-buffer textures
			renderingSystem.BindRenderTargetTexture(0, gBufferRT, Graphics::RenderTargetTexture::Color);
			renderingSystem.BindRenderTargetTexture(1, gBufferRT, Graphics::RenderTargetTexture::Depth);
			renderingSystem.BindRenderTargetTexture(2, gBufferRT, Graphics::RenderTargetTexture::Aux);

			// Prepare target RT
			renderingSystem.BindRenderTarget(tempRT);
			renderingSystem.ClearScreen(Graphics::ClearState::AllBuffers());

			// Draw fullscreen quad
			renderingSystem.UseShaderProgram(deferredLightShader);
			renderingSystem.Draw(quadVertexBuffer, Graphics::BufferHandle::Invalid(), 6);

			// Bind tempRT's color texture
			renderingSystem.BindRenderTargetTexture(0, tempRT, Graphics::RenderTargetTexture::Color);
		}

		// Bind default framebuffer
		renderingSystem.BindRenderTarget(Graphics::DefaultRenderTarget());
		renderingSystem.ClearScreen(Graphics::ClearState::AllBuffers());

		// Bind depth- and normal-textures for postprocessing (color is from lighting pass)
		renderingSystem.BindRenderTargetTexture(1, gBufferRT, Graphics::RenderTargetTexture::Depth);
		renderingSystem.BindRenderTargetTexture(2, gBufferRT, Graphics::RenderTargetTexture::Aux); // Normals

		if (ssaoState > 0)
			ssaoPP.Run(Graphics::DefaultRenderTarget());
		else
		{
			renderingSystem.UseShaderProgram(copyShader);
			renderingSystem.Draw(quadVertexBuffer, Graphics::BufferHandle::Invalid(), 6);
		}
#endif
		renderingSystem.SubmitFrame();
		++frames;

		// Print FPS
		if (timeAccum > 1.0)
		{
			printf("FPS: %f\n", frames / timeAccum);
			frames = 0;
			timeAccum = 0.0;
		}

		textureLoader.LoadOne(renderingSystem, dataFolder);

		// Poll window-events
		renderingSystem.PollEvents();

		// Enable/Disable features
		if (renderingSystem.WasPressed(Graphics::RenderingSystem::Key::F1))
		{
			ssaoState = (ssaoState + 1) % 3;
			printf("SSAO: %s\n", ssaoText[ssaoState]);
		}

		if (renderingSystem.WasPressed(Graphics::RenderingSystem::Key::F2))
		{
			normalMappingEnabled = !normalMappingEnabled;
			printf("Normal-mapping: %s\n", normalMappingEnabled ? "ON" : "OFF");
		}

		if (renderingSystem.WasPressed(Graphics::RenderingSystem::Key::F3))
		{
			parallaxMappingEnabled = !parallaxMappingEnabled;
			printf("Parallax-mapping: %s\n", parallaxMappingEnabled ? "ON" : "OFF");
		}

		// Hot reload of shaders
		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::SPACE))
			renderingSystem.ReloadShaders();

		if (renderingSystem.IsKeyDown(Graphics::RenderingSystem::Key::ESCAPE))
			break;
	}

	renderingSystem.Shutdown();

	return 0;
}