#include "Material.h"

#include "graphics/RenderingSystem.h"
#include "Constants.h"

void Material::Bind(Graphics::RenderingSystem& rs) const
{
	rs.BindUniformBuffer(Constants::MATERIAL_UBO_BINDING_INDEX, m_uniformBuffer);
	rs.BindTexture2D(Constants::MATERIAL_DIFF_TEX_UNIT, m_diffuseTextureHandle);
	rs.BindTexture2D(Constants::MATERIAL_HEIGHT_TEX_UNIT, m_heightMapTextureHandle);
	rs.BindTexture2D(Constants::MATERIAL_NORMAL_TEX_UNIT, m_normalMapTextureHandle);
}