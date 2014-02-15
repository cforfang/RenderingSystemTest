#pragma once

#include <stdint.h>
#include "glm/glm.hpp"

namespace Graphics
{
	struct ClearState
	{
		static ClearState AllBuffers() 
		{
			ClearState cs;
			cs.clearBuffers = Enum::All;
			return cs;
		}

		ClearState()
			: clearBuffers(Enum::ColorAndDepthBuffer), clearColor(glm::vec4(1, 1, 1, 1)), clearDepth(1.0f), clearStencil(0)
		{
			//depthMask = true;
			//stencilMask = 0xFF;
		}

		enum Enum
		{
			ColorBuffer = 1 << 0,
			DepthBuffer = 1 << 1,
			StencilBuffer = 1 << 2,
			ColorAndDepthBuffer = ColorBuffer | DepthBuffer,
			All = ColorBuffer | DepthBuffer | StencilBuffer
		};

		Enum clearBuffers;
		glm::vec4 clearColor;
		float     clearDepth;
		int       clearStencil;
		//ColorMask colorMask;
		//bool      depthMask;
		//int       stencilMask;
	};

	enum class PrimitiveType : uint8_t
	{
		Points,
		Lines,
		LineLoop,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan,
		LinesAdjacency,
		LineStripAdjacency,
		TriangleAdjacency,
		TriangleStripAdjacency,
		Patches,
	};

	enum class RasterizationMode
	{
		Point,
		Line,
		Fill
	};

	enum class StencilOperation
	{
		Zero,
		Invert,
		Keep,
		Replace,
		Increment,
		Decrement,
		IncrementWrap,
		DecrementWrap
	};

	enum class StencilTestFunction
	{
		Never,
		Less,
		Equal,
		LessThanOrEqual,
		Greater,
		NotEqual,
		GreaterThanOrEqual,
		Always
	};

	struct StencilTest
	{
		bool enabled = false;

		StencilTestFunction function = StencilTestFunction::Always;
		StencilOperation failOperation = StencilOperation::Keep;
		StencilOperation depthFailOperation = StencilOperation::Keep;
		StencilOperation depthPassStencilPassOperation = StencilOperation::Keep;

		uint8_t referenceValue = 0;
		uint8_t mask = 0xFF;
	};

	struct FacetCulling
	{
		enum class CullFace { Back, Front, FrontAndBack };
		enum class WindingOrder { Clockwise, Counterclockwise };

		void SwapWindingOrder()
		{
			if (frontFaceWindingOrder == WindingOrder::Clockwise)
				frontFaceWindingOrder = WindingOrder::Counterclockwise;
			else
				frontFaceWindingOrder = WindingOrder::Clockwise;
		}

		bool enabled = true;
		CullFace face = CullFace::Back;
		WindingOrder frontFaceWindingOrder = WindingOrder::Clockwise;
	};

	struct DepthTest
	{
		enum class DepthTestFunction
		{
			Never,
			Less,
			Equal,
			LessThanOrEqual,
			Greater,
			NotEqual,
			GreaterThanOrEqual,
			Always
		};

		bool enabled = true;
		DepthTestFunction function = DepthTestFunction::LessThanOrEqual;
	};

	struct Blending
	{
		enum class SourceBlendingFactor
		{
			Zero,
			One,
			SourceAlpha,
			OneMinusSourceAlpha,
			DestinationAlpha,
			OneMinusDestinationAlpha,
			DestinationColor,
			OneMinusDestinationColor,
			SourceAlphaSaturate,
			ConstantColor,
			OneMinusConstantColor,
			ConstantAlpha,
			OneMinusConstantAlpha
		};

		enum class DestinationBlendingFactor
		{
			Zero,
			One,
			SourceColor,
			OneMinusSourceColor,
			SourceAlpha,
			OneMinusSourceAlpha,
			DestinationAlpha,
			OneMinusDestinationAlpha,
			DestinationColor,
			OneMinusDestinationColor,
			ConstantColor,
			OneMinusConstantColor,
			ConstantAlpha,
			OneMinusConstantAlpha
		};

		bool enabled = true;
		SourceBlendingFactor sourceAlphaFactor = SourceBlendingFactor::SourceAlpha;
		DestinationBlendingFactor destinationAlphaFactor = DestinationBlendingFactor::OneMinusSourceAlpha;
	};

	struct ColorMask
	{
		bool red;
		bool green;
		bool blue;
		bool alpha;

		ColorMask()
		{
			red = green = blue = alpha = true;
		}

		void SetAll(bool b)
		{
			red = green = blue = alpha = b;
		}

		bool operator==(const ColorMask& rhs) const
		{
			return red == rhs.red && blue == rhs.blue && green == rhs.green && alpha == rhs.alpha;
		}

		bool operator!=(const ColorMask& rhs) const
		{
			return !operator==(rhs);
		}
	};

	struct RenderState
	{
		FacetCulling      facetCulling;
		RasterizationMode rasterizationMode = RasterizationMode::Fill;
		StencilTest       stencilTest;
		DepthTest         depthTest;
		Blending          blending;
		ColorMask         colorMask;

		float lineWidth = 1.0f;
		bool depthMask = true;
		bool depthClamp = false;
		uint8_t stencilMask = 0xFF;
	};
}
