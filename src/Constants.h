#pragma once

#include <stdint.h>

namespace Constants
{
	static const uint16_t PER_FRAME_UBO_BINDING_INDEX = 0;
	static const uint16_t PER_DRAW_UBO_BINDING_INDEX = 1;
	static const uint16_t LIGHTS_UBO_BINDING_INDEX = 2;
	static const uint16_t MATERIAL_UBO_BINDING_INDEX = 10;

	static const uint16_t MATERIAL_DIFF_TEX_UNIT = 0;
	static const uint16_t MATERIAL_NORMAL_TEX_UNIT = 1;
	static const uint16_t MATERIAL_HEIGHT_TEX_UNIT = 2;
	static const uint16_t MATERIAL_SPECULAR_TEX_UNIT = 3;
}