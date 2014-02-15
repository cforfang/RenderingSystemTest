#pragma once

#include <array>

#include "OpenGL.h"
#include "glm/glm.hpp"
#include "RenderState.h"

class UniformGroup;

namespace Graphics
{
	class ShaderProgram;
	class VertexArray;
	class Texture;
	class Buffer;

	#undef DrawState // windows.h
	class DrawState
	{
	public:
		DrawState() = default;
		DrawState(RenderState* renderState, ShaderProgram* shaderProgram, VertexArray* vertexArray, bool shadow);

		void AddTextureBinding(unsigned int location, GLuint id, GLenum target);
		void AddUniformBufferBinding(int location, GLuint id);

		GLuint m_VAO;
		GLuint m_IBO;
		GLuint m_elements;

		glm::mat4      m_transform;
		RenderState    m_renderState;

		ShaderProgram* m_shaderProgram = nullptr;
		UniformGroup*  m_uniformGroupPtr = nullptr;

		static const int MAX_UBOS = 16;
		static const int MAX_TEXTURES = 32;

		// UBOs
		unsigned int m_uboBindingNum = 0;
		std::array<std::pair<GLuint, GLuint>, MAX_UBOS> m_uboBindings; // pair<slot, uboHandle>

		// Textures
		struct TextureBinding
		{
			GLuint index;
			GLuint textureHandle;
			GLenum target;
		};
		unsigned int m_textureBindingNum = 0;
		std::array<TextureBinding, MAX_TEXTURES> m_textureBindings;
	};

}