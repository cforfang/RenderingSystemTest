#include "Buffer.h"
#include "OpenGL.h"

namespace Graphics
{
	Buffer::~Buffer()
	{
		Delete();
	}

	void Buffer::Generate()
	{
		if (m_handle != 0) return;
		glGenBuffers(1, &m_handle);
	}

	bool Buffer::IsGenerated()
	{
		return m_handle != 0;
	}

	void Buffer::Delete()
	{
		if (m_handle != 0)
		{
			glDeleteBuffers(1, &m_handle);
			m_handle = 0;
		}
	}

	void Buffer::BufferData(GLsizeiptr size, const GLvoid* data, GLenum usage)
	{
		Generate();
		glNamedBufferDataEXT(m_handle, size, data, usage);
		m_size = size;
	}

	void Buffer::BufferSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data)
	{
		Generate();
		glNamedBufferSubDataEXT(m_handle, offset, size, data);
	}
}