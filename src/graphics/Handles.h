#pragma once
#include <stdint.h>

namespace Graphics
{
#define TE_HANDLE(name) \
	struct name {  \
		uint16_t handle; \
		bool IsValid() const { \
			return handle != 0; \
		} \
		static name Invalid() { \
			return{ 0 }; \
		} \
		bool operator==(const name& rhs) \
		{ \
			return handle == rhs.handle; \
		} \
		bool operator!=(const name& rhs) \
		{ \
			return !(handle == rhs.handle); \
		} \
	};

	TE_HANDLE(ShaderProgramHandle);
	TE_HANDLE(BufferHandle);
	TE_HANDLE(VertexArrayHandle);
	TE_HANDLE(Texture2DHandle);

	TE_HANDLE(RenderTargetHandle);
	inline const RenderTargetHandle DefaultRenderTarget() { return Graphics::RenderTargetHandle::Invalid(); };
}