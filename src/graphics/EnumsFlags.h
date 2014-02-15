#pragma once

#include <stdint.h>

namespace Graphics
{
	enum class BufferType : uint8_t
	{
		STATIC,
		DYNAMIC
	};

	enum class TextureType : uint8_t 
	{ SRGBA8, RGBA8, R8, Depth, None };

	enum class RenderTargetTexture : uint8_t
	{
		Color, Depth, Aux
	};

	struct RenderTargetOptions
	{
		static RenderTargetOptions SRGB8Depth(int w, int h) 
		{ 
			RenderTargetOptions options;
			options.colorTexture = TextureType::SRGBA8;
			options.depthTexture = TextureType::Depth;
			options.auxTexture = TextureType::None;
			options.width = w;
			options.height = h;
			return options;
		}

		static RenderTargetOptions SRGB8DepthRGB8(int w, int h)
		{ 
			RenderTargetOptions options;
			options.colorTexture = TextureType::SRGBA8;
			options.depthTexture = TextureType::Depth;
			options.auxTexture = TextureType::RGBA8;
			options.width = w;
			options.height = h;
			return options;
		}

		TextureType colorTexture = TextureType::None;
		TextureType depthTexture = TextureType::None;
		TextureType auxTexture = TextureType::None;

		int width;
		int height;
	};
}
