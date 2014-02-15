#pragma once

#include <array>

#include "EnumsFlags.h"
#include "Handles.h"
#include "Buffer.h"
#include "RenderState.h"
#include "ShaderProgram.hpp"

namespace Graphics
{
	struct CommandBuffer;

	class Context
	{
	public:
		Context();
		~Context();

		void Init();
		void ExecuteCommandBuffer(CommandBuffer* ptr);

	private:
		void Clear(const ClearState& clearState);
		void CreateShaderProgram(const ShaderProgramHandle& handle, const ShaderInfo& si);
		void UseShaderProgram(const ShaderProgramHandle& handle);
		void CreateBuffer(const BufferHandle& buffer);
		void UpdateBuffer(const BufferHandle& buffer, void* data, uint32_t size, GLenum usage);
		void CreateTexture2D(const Texture2DHandle& buffer);
		void UpdateTexture2D(const Texture2DHandle& tex, void* data, uint16_t width, uint16_t height, TextureType type);
		void BindTexture2D(uint8_t unit, const Texture2DHandle& tex);
		void Draw(const BufferHandle& v, const BufferHandle& i, uint32_t elements);
		void BindUniformBuffer(uint8_t index, BufferHandle buffer);
		void CreateRenderTarget(const RenderTargetHandle& handle, const RenderTargetOptions& options);
		void BindRenderTarget(const RenderTargetHandle& handle);
		void BindRenderTargetTexture(uint8_t unit, const RenderTargetHandle& handle, const RenderTargetTexture& texture);

		void _BindTexture2D(uint8_t unit, GLuint tex);

		ClearState m_currentClearState;

		static const int MAX_SHADERS = 4096;
		std::array<ShaderProgram, 4096> m_shaderPrograms;

		static const int MAX_BUFFERS = 32000;
		std::array<Graphics::Buffer, MAX_BUFFERS> m_buffers;

		static const int MAX_TEXTURES = 4096;
		std::array<GLuint, MAX_TEXTURES> m_texture2Ds;

		GLuint m_defaultVAO = 0;

		std::array<GLuint, 32> m_boundTextures;
		std::array<GLuint, 32> m_boundUniformBuffers;

		struct RenderTarget
		{
			GLuint fbo;
			GLuint colorTexture;
			GLuint depthTexture;
			GLuint auxTexture;
		};

		static const int MAX_RENDERTARGETS = 4096;
		std::array<RenderTarget, MAX_RENDERTARGETS> m_renderTargets;
	};
};