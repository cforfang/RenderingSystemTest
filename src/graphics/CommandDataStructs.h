#pragma once

#include "Handles.h"
#include "RenderState.h"

// Used as to avoid having missmatches between reads and writes of commands to the CommandBuffer.
// (E.g. accidentally writing an uint8_t and reading an uint16_t.)

namespace Graphics
{
	struct ShaderInfo;

	struct DrawData
	{
		BufferHandle vertexBuffer;
		BufferHandle indexBuffer;
		uint32_t elements;
	};

	struct ClearScreenData
	{
		Graphics::ClearState clearState;
	};

	struct CreateShaderProgramData
	{
		Graphics::ShaderInfo* siPtr;
		ShaderProgramHandle handle;
	};

	struct CreateBufferData
	{
		BufferHandle handle;
	};

	struct UpdateBufferData
	{
		BufferHandle buffer;
		void* data;
		uint32_t size;
		BufferType usage;
	};

	struct CreateTexture2DData
	{
		Texture2DHandle handle;
	};

	struct BindTexture2DData
	{
		uint8_t unit;
		Texture2DHandle texture;
	};

	struct UseShaderProgramData
	{
		ShaderProgramHandle handle;
	};

	struct UploadTexture2DData
	{
		Texture2DHandle buffer;
		void* data;
		uint16_t width;
		uint16_t height;
		TextureType type;
	};

	struct BindUniformBufferData
	{
		uint8_t bindingIndex;
		BufferHandle buffer;
	};

	struct CreateRenderTargetData
	{
		RenderTargetHandle handle;
		RenderTargetOptions options;
	};

	struct BindRenderTargetData
	{
		RenderTargetHandle handle;
	};

	struct BindRenderTargetTexturesData
	{
		uint8_t unit;
		RenderTargetHandle handle;
		RenderTargetTexture texture;
	};
}