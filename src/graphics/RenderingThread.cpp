#include "RenderingThread.h"

#include "OpenGL.h"
#include "CommandBuffer.h"
#include "Context.h"

#include <iostream>
#include <cstring> // memcpy

namespace Graphics
{
	RenderingThread::RenderingThread()
	{

	}

	RenderingThread::~RenderingThread()
	{
		// Must call join() on thread
		Shutdown();
	}

	bool RenderingThread::Init(GLFWwindow* window)
	{
		m_shouldExit = false;
		m_windowHandle = window;

		m_thread = std::thread([this]
		{
			// Take context
			glfwMakeContextCurrent(m_windowHandle);

			{
				std::unique_ptr<Graphics::Context> renderingContext(new Graphics::Context);
				renderingContext->Init();

				printf("---RENDERING_THREAD---\n");
				printf("OpenGL %s\n", glGetString(GL_VERSION));
				printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
				printf("----------------------\n");

				m_threadIsReady.notify();

				while (!m_shouldExit)
				{
					m_threadStart.wait();

					if (m_shouldExit)
						break;

					if (m_commandBuffer)
					{
						renderingContext->ExecuteCommandBuffer(m_commandBuffer);
						glfwSwapBuffers(m_windowHandle);
						m_commandBuffer = nullptr;
					}

					m_threadIsReady.notify();
				}
			}

			// Release context when exiting
			glfwMakeContextCurrent(NULL);
		});

		return true;
	}

	void RenderingThread::Shutdown()
	{
		if (m_shouldExit)
			return;

		m_threadIsReady.wait();
		m_shouldExit = true;
		m_threadStart.notify();

		m_thread.join();
	}

	void RenderingThread::WaitAndExecute(Graphics::CommandBuffer* cmdBuff)
	{
		m_threadIsReady.wait();
		m_commandBuffer = cmdBuff;
		m_threadStart.notify();
	}
}