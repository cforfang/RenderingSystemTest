#pragma once

#include <memory>
#include <string>

#include "RenderState.h"
#include "ShaderInfo.h"
#include "Handles.h"
#include "EnumsFlags.h"

namespace Graphics
{
	struct ContextConfig
	{
		unsigned int colorBits = 24;
		unsigned int depthBits = 24;
		unsigned int stencilBits = 8;
		unsigned int msaaSamples = 4;

		bool debugContext = false;
		bool coreProfileContext = true;
		bool synchronousDebugOutput = false;
		bool glewExperimental = true;
	};

	enum class WindowMode
	{
		WINDOW, FULLSCREEN
	};

	struct WindowConfig
	{
		std::string title;
		int width;
		int height;
		WindowMode mode;
		bool resizable;
	};

	class RenderingSystem_impl;

	class RenderingSystem
	{
	public:
		RenderingSystem();
		~RenderingSystem();

		bool Init(const WindowConfig& wc, const ContextConfig& cc);
		void Shutdown();

		// Return time since init
		double GetTime();

		/* Operations on window */
		bool CloseRequested();
		void PollEvents();

		void SetWindowTitle(const std::string& title);

		enum class Key { SPACE, ESCAPE, W, A, S, D, Q, E, SHIFT, F1, F2, F3, LAST_KEY /* to track enum legth */ };
		bool IsKeyDown(Key key);
		bool WasPressed(Key key);

		enum class MouseButton { Right, Left };
		bool IsMouseButtonDown(MouseButton key);
		
		glm::vec2 GetCursorPosition();
		void SetCursorEnabled(bool enabled);

		/* Below: operations on the current (to be rendered) frame */

		// Screen
		void ClearScreen(const Graphics::ClearState& clearState);

		// Shaderprograms
		ShaderProgramHandle CreateShaderProgram(const Graphics::ShaderInfo& si);
		void ReloadShaders();
		void UseShaderProgram(ShaderProgramHandle handle);

		// Buffers
		BufferHandle CreateBuffer();
		void UpdateBuffer(BufferHandle buffer, void* data, uint32_t size, BufferType usage);
		void BindUniformBuffer(uint8_t bindingIndex, BufferHandle buffer);

		// Textures
		Texture2DHandle CreateTexture2D();
		void UpdateTexture2D(Texture2DHandle buffer, void* data, uint16_t width, uint16_t height, TextureType flags);
		void BindTexture2D(uint8_t unit, Texture2DHandle buffer);

		// Rendertargets
		RenderTargetHandle CreateRenderTarget(RenderTargetOptions textures);
		void BindRenderTarget(RenderTargetHandle handle);
		void BindRenderTargetTexture(uint8_t unit, RenderTargetHandle handle, RenderTargetTexture texture);

		// Drawing
		void Draw(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t elements);

		// Submit current frame for rendering
		void SubmitFrame();

	private:
		struct RenderingSystem_data;
		std::unique_ptr<RenderingSystem_data> m_data;
	};
}