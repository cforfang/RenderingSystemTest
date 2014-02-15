#pragma once

#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

struct GLFWwindow;

namespace Graphics
{
	struct CommandBuffer;

	class Semaphore {
	private:
		std::mutex mtx;
		std::condition_variable cv;
		int count;

	public:
		Semaphore(int count_ = 0) : count(count_)
		{ }

		void notify()
		{
			std::unique_lock<std::mutex> lck(mtx);
			++count;
			cv.notify_one();
		}

		void wait()
		{
			std::unique_lock<std::mutex> lck(mtx);
			while (count == 0){
				cv.wait(lck);
			}
			count--;
		}
	};

	class RenderingThread
	{
	public:
		RenderingThread();
		~RenderingThread();

		bool Init(GLFWwindow* window);
		void Shutdown();

		// Waits until current frame is drawn, 
		// then hands over new commandbuffer.
		void WaitAndExecute(CommandBuffer* commandBuffer);

		Semaphore m_threadIsReady;
		Semaphore m_threadStart;

	private:
		CommandBuffer* m_commandBuffer;
		GLFWwindow* m_windowHandle;

		std::thread m_thread;
		std::atomic<bool> m_shouldExit;
	};
}