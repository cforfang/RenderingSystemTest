#include "Context.h"
#include "OpenGL.h"

#include "CommandBuffer.h"
#include "ShaderProgram.hpp"
#include "Handles.h"
#include "EnumsFlags.h"
#include "CommandDataStructs.h"

#include <iostream>

namespace Graphics
{
	namespace
	{
		GLenum toGL(TextureType texType)
		{
			switch (texType)
			{
			case TextureType::RGBA8:
				return GL_RGBA8;
			case TextureType::R8:
				return GL_R8;
			case TextureType::SRGBA8:
				return GL_SRGB8_ALPHA8;
			case TextureType::Depth:
				return GL_DEPTH_COMPONENT;
			default:
				assert(false && "toGL(TextureType) with invalid texturetype");
				return 0;
			}
		}
	}
	Context::Context()
	{
		//ForceKnownState();
	}

	Context::~Context()
	{
		for (auto& rt : m_renderTargets)
		{
			glDeleteTextures(1, &rt.colorTexture);
			glDeleteTextures(1, &rt.auxTexture);
			glDeleteTextures(1, &rt.depthTexture);
			glDeleteFramebuffers(1, &rt.fbo);
			rt = { 0u, 0u, 0u, 0u };
		}

		for (auto& tex : m_texture2Ds)
		{
			glDeleteTextures(1, &tex);
			tex = 0;
		}

		glDeleteVertexArrays(1, &m_defaultVAO);
	}

	void Context::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glEnable(GL_FRAMEBUFFER_SRGB);

		m_texture2Ds.fill(0u);
		m_boundUniformBuffers.fill(0u);

		RenderTarget rt = { 0u, 0u, 0u, 0u };
		m_renderTargets.fill(rt);
	}

	void Context::ExecuteCommandBuffer(CommandBuffer* cmdBuffer)
	{
		cmdBuffer->reset();

		bool end = false;
		CommandBuffer::Command command;

		do
		{
			cmdBuffer->read(command);

			switch (command)
			{
			case CommandBuffer::Command::ClearScreen:
			{
				ClearScreenData data;
				cmdBuffer->read(data);
				Clear(data.clearState);
				break;
			}
			case CommandBuffer::Command::CreateShaderProgram:
			{
				CreateShaderProgramData data;
				cmdBuffer->read(data);

				CreateShaderProgram(data.handle, *data.siPtr);

				delete data.siPtr;
				break;
			}
			case CommandBuffer::Command::CreateBuffer:
			{
				CreateBufferData data;
				cmdBuffer->read(data);
				CreateBuffer(data.handle);
				break;
			}
			case CommandBuffer::Command::UpdateBuffer:
			{
				UpdateBufferData data;
				cmdBuffer->read(data);

				GLenum glUsage = (data.usage == BufferType::STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

				UpdateBuffer(data.buffer, data.data, data.size, glUsage);
				break;
			}
			case CommandBuffer::Command::CreateTexture2D:
			{
				CreateTexture2DData data;
				cmdBuffer->read(data);
				CreateTexture2D(data.handle);
				break;
			}
			case CommandBuffer::Command::UploadTexture2D:
			{
				UploadTexture2DData data;
				cmdBuffer->read(data);
				UpdateTexture2D(data.buffer, data.data, data.width, data.height, data.type);
				break;
			}
			case CommandBuffer::Command::BindTexture2D:
			{
				BindTexture2DData data;
				cmdBuffer->read(data);
				BindTexture2D(data.unit, data.texture);
				break;
			}
			case CommandBuffer::Command::UseShaderProgram:
			{
				UseShaderProgramData data;
				cmdBuffer->read(data);
				UseShaderProgram(data.handle);
				break;
			}
			case CommandBuffer::Command::Draw:
			{
				DrawData data;
				cmdBuffer->read(data);
				Draw(data.vertexBuffer, data.indexBuffer, data.elements);
				break;
			}
			case CommandBuffer::Command::BindUniformBuffer:
			{
				BindUniformBufferData data;
				cmdBuffer->read(data);
				BindUniformBuffer(data.bindingIndex, data.buffer);
				break;
			}
			case CommandBuffer::Command::CreateRenderTarget:
			{
				CreateRenderTargetData data;
				cmdBuffer->read(data);
				CreateRenderTarget(data.handle, data.options);
				break;
			}
			case CommandBuffer::Command::BindRenderTarget:
			{
				BindRenderTargetData data;
				cmdBuffer->read(data);
				BindRenderTarget(data.handle);
				break;
			}
			case CommandBuffer::Command::BindRenderTargetTextures:
			{
				BindRenderTargetTexturesData data;
				cmdBuffer->read(data);
				BindRenderTargetTexture(data.unit, data.handle, data.texture);
				break;
			}
			case CommandBuffer::Command::ReloadShaders:
			{
				for (auto& shader : m_shaderPrograms)
				{
					shader.Reload();
				}
				
				break;
			}
			case CommandBuffer::Command::End: 
			{
				end = true;
				break;
			}
			default:
				assert(false && "Invalid command parsed in Context::ExecuteCommandBuffer");
				break;
			}
		} while (!end);
	}

	void Context::CreateShaderProgram(const ShaderProgramHandle& handle, const ShaderInfo& si)
	{
		assert(handle.handle < MAX_SHADERS);
		if (!m_shaderPrograms[handle.handle].Load(si))
		{
			// Fail-fast on shader-compilation errors
			exit(1); 
		}
	}

	void Context::UseShaderProgram(const ShaderProgramHandle& handle)
	{
		ShaderProgram& program = m_shaderPrograms[handle.handle];

		if (program.IsLoaded())
			program.UseProgram();
		else
			glUseProgram(0);
	}

	void Context::CreateBuffer(const BufferHandle& handle)
	{
		assert(handle.handle < MAX_BUFFERS);
		m_buffers[handle.handle].Generate(); 
	}

	void Context::UpdateBuffer(const BufferHandle& bufferHandle, void* data, uint32_t size, GLenum usage)
	{
		assert(bufferHandle.handle < MAX_BUFFERS);
		assert(bufferHandle.IsValid());

		Buffer& buffer = m_buffers[bufferHandle.handle];
		assert(buffer.IsGenerated());

		buffer.BufferData(size, NULL, usage); // Orphan
		buffer.BufferData(size, data, usage);

		free(data);
	}

	void Context::BindTexture2D(uint8_t unit, const Texture2DHandle& tex)
	{
		assert(tex.handle < MAX_TEXTURES);
		GLuint toBind = m_texture2Ds[tex.handle];
		_BindTexture2D(unit, toBind);
	}

	void Context::_BindTexture2D(uint8_t unit, GLuint tex)
	{
		if (m_boundTextures[unit] != tex) 
		{
			m_boundTextures[unit] = tex;
			glBindMultiTextureEXT(GL_TEXTURE0 + unit, GL_TEXTURE_2D, tex);
		}
	}

	void Context::CreateRenderTarget(const RenderTargetHandle& handle, const RenderTargetOptions& options)
	{
		assert(handle.handle < MAX_RENDERTARGETS);
		auto& rt = m_renderTargets[handle.handle];

		assert(rt.fbo == 0);

		glGenFramebuffers(1, &rt.fbo);

		GLenum drawBuffers[] = { GL_NONE, GL_NONE };

		if (options.colorTexture != TextureType::None)
		{
			glGenTextures(1, &rt.colorTexture);
			glTextureImage2DEXT(rt.colorTexture, GL_TEXTURE_2D, 0, toGL(options.colorTexture), options.width, options.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTextureParameteriEXT(rt.colorTexture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(rt.colorTexture, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glNamedFramebufferTextureEXT(rt.fbo, GL_COLOR_ATTACHMENT0, rt.colorTexture, 0);
			drawBuffers[0] = GL_COLOR_ATTACHMENT0;
		}

		if (options.depthTexture != TextureType::None)
		{
			glGenTextures(1, &rt.depthTexture);
			glTextureImage2DEXT(rt.depthTexture, GL_TEXTURE_2D, 0, toGL(options.depthTexture), options.width, options.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTextureParameteriEXT(rt.depthTexture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(rt.depthTexture, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glNamedFramebufferTextureEXT(rt.fbo, GL_DEPTH_ATTACHMENT, rt.depthTexture, 0);
		}		

		if (options.auxTexture != TextureType::None)
		{
			glGenTextures(1, &rt.auxTexture);
			glTextureImage2DEXT(rt.auxTexture, GL_TEXTURE_2D, 0, toGL(options.auxTexture), options.width, options.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTextureParameteriEXT(rt.auxTexture, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteriEXT(rt.auxTexture, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glNamedFramebufferTextureEXT(rt.fbo, GL_COLOR_ATTACHMENT1, rt.auxTexture, 0);

			drawBuffers[1] = GL_COLOR_ATTACHMENT1;
		}

		glFramebufferDrawBuffersEXT(rt.fbo, 2, drawBuffers);

		GLenum status = glCheckNamedFramebufferStatusEXT(rt.fbo, GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			fprintf(stderr, "Framebuffer with handle %d not complete.\n", handle.handle);
		}
	}

	void Context::BindRenderTarget(const RenderTargetHandle& handle)
	{
		assert(handle.handle < MAX_RENDERTARGETS);
		if (!handle.IsValid())
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			return;
		}

		auto& rt = m_renderTargets[handle.handle];
		glBindFramebuffer(GL_FRAMEBUFFER, rt.fbo);
	}

	void Context::BindRenderTargetTexture(uint8_t unit, const RenderTargetHandle& handle, const RenderTargetTexture& texture)
	{
		assert(handle.handle < MAX_RENDERTARGETS);
		auto& rt = m_renderTargets[handle.handle];

		GLuint toBind = 0;

		switch (texture)
		{
		case RenderTargetTexture::Color:
			toBind = rt.colorTexture;
			break;
		case RenderTargetTexture::Depth:
			toBind = rt.depthTexture;
			break;
		case RenderTargetTexture::Aux:
			toBind = rt.auxTexture;
			break;
		}

		_BindTexture2D(unit, toBind);
	}

	void Context::Draw(const BufferHandle& v, const BufferHandle& i, uint32_t elements)
	{
		if (m_defaultVAO == 0)
			glGenVertexArrays(1, &m_defaultVAO);

		glBindVertexArray(m_defaultVAO);

		assert(v.handle < MAX_BUFFERS);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[v.handle].GetHandle());

		const uint32_t stride = 14 * sizeof(float);

		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		// Texcoords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// Normal
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// Tangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);

		// Bitangent 
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(11 * sizeof(float)));
		glEnableVertexAttribArray(4);

		if (i.handle != 0)
		{
			assert(i.handle < MAX_BUFFERS);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[i.handle].GetHandle());
			glDrawElements(GL_TRIANGLES, elements, GL_UNSIGNED_INT, 0);
		}
		else 
		{
			glDrawArrays(GL_TRIANGLES, 0, elements);
		}
	}

	void Context::BindUniformBuffer(uint8_t index, BufferHandle buffer)
	{
		assert(buffer.handle < MAX_BUFFERS);
		GLuint bufferGLuint = m_buffers[buffer.handle].GetHandle();

		if (m_boundUniformBuffers[index] != bufferGLuint)
		{
			m_boundUniformBuffers[index] = bufferGLuint;
			glBindBufferBase(GL_UNIFORM_BUFFER, index, bufferGLuint);
		}
	}

	void Context::CreateTexture2D(const Texture2DHandle& tex)
	{
		assert(tex.handle < MAX_TEXTURES);
		auto& glUint = m_texture2Ds[tex.handle];

		if (glUint == 0)
			glGenTextures(1, &glUint);
	}

	void Context::UpdateTexture2D(const Texture2DHandle& tex, void* data, uint16_t width, uint16_t height, TextureType type)
	{
		assert(tex.handle < MAX_TEXTURES);
		auto& glUint = m_texture2Ds[tex.handle];

		assert(glUint != 0);

		GLenum internalFormat = toGL(type);

		glTextureImage2DEXT(
			glUint,
			GL_TEXTURE_2D,
			0, // Level
			internalFormat,
			width, height,
			0, // Border (must be 0)
			GL_RGBA, // Format
			GL_UNSIGNED_BYTE, // Buffer type
			data);

		// Not done correctly for SRGB-textures with AMD driver 13.20.16-130926a-163066E-ATI
		glGenerateTextureMipmapEXT(glUint, GL_TEXTURE_2D);

		glTextureParameteriEXT(glUint, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTextureParameteriEXT(glUint, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameterfEXT(glUint, GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 32.0f);

		free(data);
	}

	void Context::Clear(const ClearState& clearState)
	{
		//ApplyColorMask(clearState.colorMask);
		//ApplyDepthMask(clearState.depthMask);
		//ApplyStencilMask(clearState.stencilMask);

		if (m_currentClearState.clearDepth != clearState.clearDepth)
		{
			m_currentClearState.clearDepth = clearState.clearDepth;
			glClearDepth(m_currentClearState.clearDepth);
		}

		if (m_currentClearState.clearStencil != clearState.clearStencil)
		{
			m_currentClearState.clearStencil = clearState.clearStencil;
			glClearStencil(m_currentClearState.clearStencil);
		}

		if (m_currentClearState.clearColor != clearState.clearColor)
		{
			m_currentClearState.clearColor = clearState.clearColor;
			glClearColor(clearState.clearColor.x, clearState.clearColor.y, clearState.clearColor.z, clearState.clearColor.w);
		}

		m_currentClearState = clearState;

		switch (clearState.clearBuffers)
		{
		case ClearState::Enum::ColorBuffer:
			glClear(GL_COLOR_BUFFER_BIT);
			break;
		case ClearState::Enum::DepthBuffer:
			glClear(GL_DEPTH_BUFFER_BIT);
			break;
		case ClearState::Enum::StencilBuffer:
			glClear(GL_STENCIL_BUFFER_BIT);
			break;
		case ClearState::Enum::ColorAndDepthBuffer:
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			break;
		case ClearState::Enum::All:
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			break;
		default:
			printf("Called Context::Clear without a valid buffers-to-clear setting.\n");
		}
	}
}