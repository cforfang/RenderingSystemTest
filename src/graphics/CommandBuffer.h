#pragma once

#include <stdint.h>
#include <cassert>
#include <cstring> // memcpy

namespace Graphics
{
        // Inspired by https://github.com/bkaradzic/bgfx/blob/master/src/bgfx_p.h line ~486
	struct CommandBuffer
	{
		enum class Command : uint8_t
		{
			CreateTexture2D,
			UploadTexture2D,
			BindTexture2D,
			CreateBuffer,
			UpdateBuffer,
			CreateShaderProgram,
			CreateRenderTarget,
			BindRenderTarget,
			BindRenderTargetTextures,
			ReloadShaders,
			UseShaderProgram,
			Draw,
			BindUniformBuffer,
			ClearScreen,
			End
		};

		static const int MAX_SIZE = 1 << 20;

		CommandBuffer() : m_pos(0), m_size(MAX_SIZE)
		{
			finish();
		}

		void write(const void* _data, uint32_t _size)
		{
			assert(m_size == MAX_SIZE && "Called write outside start/finish?");
			assert(m_pos < m_size && "cmdbuff::write");
			memcpy(&m_buffer[m_pos], _data, _size);
			m_pos += _size;
		}

		void read(void* _data, uint32_t _size)
		{
			assert(m_pos < m_size && "cmdbuff::read");
			memcpy(_data, &m_buffer[m_pos], _size);
			m_pos += _size;
		}

		template<typename Type>
		void write(const Type& _in)
		{
			write(reinterpret_cast<const uint8_t*>(&_in), sizeof(Type));
		}

		template<typename Type>
		void read(Type& _in)
		{
			read(reinterpret_cast<uint8_t*>(&_in), sizeof(Type));
		}

		const uint8_t* skip(uint32_t _size);

		void reset()
		{
			m_pos = 0;
		}

		void start()
		{
			m_pos = 0;
			m_size = MAX_SIZE;
		}

		void finish()
		{
			uint8_t cmd = (uint8_t)Command::End;
			write(cmd);
			m_size = m_pos;
			m_pos = 0;
		}

		uint32_t m_pos;
		uint32_t m_size;
		uint8_t  m_buffer[MAX_SIZE];

	private:
		CommandBuffer(const CommandBuffer&) = delete;
		void operator=(const CommandBuffer&) = delete;
	};
}