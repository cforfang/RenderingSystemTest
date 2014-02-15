#if 0
#include "DrawState.h"
#include "../Renderer/Context.h"
#include "../Renderer/Texture.hpp"
#include "Buffer.h"

#include "../UniformGroup.h"
#include "ShaderProgram.hpp"
#include "../Renderer/VertexArray.h"

namespace Renderer
{
	DrawState::DrawState(RenderState* renderState, ShaderProgram* shaderProgram, VertexArray* vertexArray, bool shadow)
		: m_uboBindingNum(0), m_textureBindingNum(0)
	{
		m_VAO = vertexArray->GetVAOHandle();
		if (shadow) m_VAO = vertexArray->GetDepthOnlyVAOHandle();

		m_IBO = vertexArray->GetIndexBuffer() ? vertexArray->GetIndexBuffer()->GetBuffer()->GetHandle() : 0;
		m_elements = vertexArray->GetNumElements();
		//m_vertexArray = vertexArray;
		m_renderState = *renderState;
		m_shaderProgram = shaderProgram;
	}

	void DrawState::AddTextureBinding(unsigned int location, Texture* tex)
	{
		DrawState::AddTextureBinding(location, tex->GetHandle(), tex->GetGLTarget());
	}

	void DrawState::AddTextureBinding(unsigned int location, GLuint id, GLenum target)
	{
		assert(location < MAX_TEXTURES);

		// Look for existing
		for (unsigned i = 0; i < m_textureBindingNum; ++i)
		{
			if (m_textureBindings[i].index == location)
			{
				m_textureBindings[i].target = target;
				m_textureBindings[i].textureHandle = id;
				return;
			}
		}

		// Add new
		m_textureBindings[m_textureBindingNum] = { static_cast<GLuint>(location), id, target };
		m_textureBindingNum++;
	}

	void DrawState::AddUniformBufferBinding(int location, Buffer* buff)
	{
		AddUniformBufferBinding(location, buff->GetHandle());
	}
	void DrawState::AddUniformBufferBinding(int location, GLuint id)
	{
		assert(location < MAX_UBOS);
		m_uboBindings[m_uboBindingNum] = std::make_pair(location, id);
		m_uboBindingNum++;
	}

}

#endif