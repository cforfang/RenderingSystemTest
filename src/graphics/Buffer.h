#pragma once

#include "OpenGL.h"

namespace Graphics
{
	class Buffer
	{
	public:
		Buffer() = default;
		virtual ~Buffer();

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;

		void Generate();
		bool IsGenerated();

		void BufferData(GLsizeiptr size, const GLvoid* data, GLenum usage);
		void BufferSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data);
		void Delete();	

		GLuint GetHandle() const { return m_handle; }
		size_t GetSize() const { return m_size; }

	private:
		size_t m_size = 0;
		GLuint m_handle = 0;
	};
}