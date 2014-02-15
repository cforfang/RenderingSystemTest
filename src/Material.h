#pragma once

#include "glm/glm.hpp"

#include "graphics/ForwardDecl.h"
#include "graphics/Handles.h"

class Material
{
public:
	Material(	const Graphics::Texture2DHandle& diffuse, 
				const Graphics::Texture2DHandle& normal,
				const Graphics::Texture2DHandle& height,
				const Graphics::BufferHandle& uniformBuffer)
				: m_diffuseTextureHandle(diffuse), m_normalMapTextureHandle(normal), m_heightMapTextureHandle(height), m_uniformBuffer(uniformBuffer)
	{};

	// Default constructor has all handles as invalid; see below
	Material()
	{};

	void Bind(Graphics::RenderingSystem& rs) const;

	const Graphics::Texture2DHandle& GetDiffuseTexture() const
	{
		return m_diffuseTextureHandle;
	}

	const Graphics::Texture2DHandle& GetHeightMapTexture() const
	{
		return m_heightMapTextureHandle;
	}

	const Graphics::Texture2DHandle& GetNormalMapTexture() const
	{
		return m_normalMapTextureHandle;
	}

	const Graphics::BufferHandle& GetUniformBuffer() const
	{
		return m_uniformBuffer;
	}

	// For sorting by material (to reduce state-changes on draw)
	friend bool operator<(const Material& lhs, const Material& rhs);

private:
	Graphics::Texture2DHandle m_diffuseTextureHandle   = Graphics::Texture2DHandle::Invalid();
	Graphics::Texture2DHandle m_normalMapTextureHandle = Graphics::Texture2DHandle::Invalid();
	Graphics::Texture2DHandle m_heightMapTextureHandle = Graphics::Texture2DHandle::Invalid();
	Graphics::BufferHandle    m_uniformBuffer          = Graphics::BufferHandle::Invalid();
};

inline bool operator<(const Material& lhs, const Material& rhs)
{
	// Same materials have same UBO-handle
	return lhs.m_uniformBuffer.handle < rhs.m_uniformBuffer.handle;
}