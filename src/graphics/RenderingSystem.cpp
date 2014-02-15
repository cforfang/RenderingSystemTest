#include "RenderingSystem.h"

#include "OpenGL.h"
#include "RenderingThread.h"
#include "CommandBuffer.h"
#include "Context.h"
#include "ShaderProgram.hpp"
#include "CommandBuffer.h"
#include "EnumsFlags.h"
#include "CommandDataStructs.h"

#define TE_MULTI_THREADED 1

static void APIENTRY OGLDebugFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
static void GLFWErrorCallback(int error, const char* message);

namespace Graphics
{
	namespace
	{
		int toGLFW(RenderingSystem::Key k)
		{
			int glfwKey = -1;

			switch (k)
			{
			case RenderingSystem::Key::SPACE: glfwKey = GLFW_KEY_SPACE; break;
			case RenderingSystem::Key::ESCAPE: glfwKey = GLFW_KEY_ESCAPE; break;
			case RenderingSystem::Key::W: glfwKey = GLFW_KEY_W; break;
			case RenderingSystem::Key::A: glfwKey = GLFW_KEY_A; break;
			case RenderingSystem::Key::S: glfwKey = GLFW_KEY_S; break;
			case RenderingSystem::Key::D: glfwKey = GLFW_KEY_D; break;
			case RenderingSystem::Key::Q: glfwKey = GLFW_KEY_Q; break;
			case RenderingSystem::Key::E: glfwKey = GLFW_KEY_E; break;
			case RenderingSystem::Key::F1: glfwKey = GLFW_KEY_F1; break;
			case RenderingSystem::Key::F2: glfwKey = GLFW_KEY_F2; break;
			case RenderingSystem::Key::F3: glfwKey = GLFW_KEY_F3; break;
			case RenderingSystem::Key::SHIFT: glfwKey = GLFW_KEY_LEFT_SHIFT; break;
			default:
				fprintf(stderr, "toGLFW(RenderingSystem::Key k): Invalid key\n");
			};

			return glfwKey;
		}
	}

	struct RenderingSystem::RenderingSystem_data
	{
		CommandBuffer& GetCurrentCommandBuffer()
		{
			return m_commandBuffers[m_currentCommandBuffer];
		};

		GLFWwindow* m_windowHandle;

		int m_currentCommandBuffer = 0;
		std::array<CommandBuffer, 2> m_commandBuffers;

		uint16_t m_numShaderPrograms = 0;
		uint16_t m_numBuffers = 0;
		uint16_t m_numTexture2D = 0;
		uint16_t m_numRenderTargets = 0;

		bool m_keyState[static_cast<int>(Key::LAST_KEY)];
		bool m_oldKeyState[static_cast<int>(Key::LAST_KEY)];

#if TE_MULTI_THREADED
		RenderingThread m_renderingThread;
#else
		Graphics::Context m_renderingContext;
#endif
	};

	RenderingSystem::RenderingSystem()
	{

	}

	RenderingSystem::~RenderingSystem()
	{

	}

	bool RenderingSystem::Init(const WindowConfig& wc, const ContextConfig& cc)
	{
		m_data.reset(new RenderingSystem_data);

		glfwSetErrorCallback(GLFWErrorCallback);

		if (!glfwInit())
		{
			fprintf(stderr, "glfwInit failed\n");
			return false;
		}

		const unsigned int colorBits = std::min(cc.colorBits, 24u);
		glfwWindowHint(GLFW_RED_BITS, colorBits / 3);
		glfwWindowHint(GLFW_GREEN_BITS, colorBits / 3);
		glfwWindowHint(GLFW_BLUE_BITS, colorBits / 3);
		glfwWindowHint(GLFW_DEPTH_BITS, cc.depthBits);
		glfwWindowHint(GLFW_STENCIL_BITS, cc.stencilBits);
		glfwWindowHint(GLFW_SAMPLES, cc.msaaSamples);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		if (cc.coreProfileContext)
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		else
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, cc.debugContext ? GL_TRUE : GL_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, wc.resizable ? GL_TRUE : GL_FALSE);
		
		GLFWmonitor* monitor = NULL;

		if (wc.mode == WindowMode::FULLSCREEN)
			monitor = glfwGetPrimaryMonitor();

		m_data->m_windowHandle = glfwCreateWindow(wc.width, wc.height, wc.title.c_str(), monitor, NULL);

		if (!m_data->m_windowHandle)
		{
			fprintf(stderr, "glfwCreateWindow failed\n");
			return false;
		}

		glfwMakeContextCurrent(m_data->m_windowHandle);

		if (cc.glewExperimental)
			glewExperimental = GL_TRUE;

		if (glewInit() != GLEW_OK)
		{
			fprintf(stderr, "glfwInit failed\n");
			return false;
		}

		if (cc.debugContext && GLEW_ARB_debug_output)
		{
			if (cc.synchronousDebugOutput)
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

			glDebugMessageCallbackARB(OGLDebugFunc, NULL);
#ifdef _DEBUG
			glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
		}

#if TE_MULTI_THREADED
		// Context is moved to rendering thread
		glfwMakeContextCurrent(NULL);
		m_data->m_renderingThread.Init(m_data->m_windowHandle);
#else
		m_data->m_renderingContext.Init();
		printf("-----MAIN_THREAD-----\n");
		printf("OpenGL %s\n", glGetString(GL_VERSION));
		printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
		printf("---------------------\n");
#endif

		m_data->GetCurrentCommandBuffer().start();

		return true;
	}

	void RenderingSystem::Shutdown()
	{
		SubmitFrame();

#if TE_MULTI_THREADED
		m_data->m_renderingThread.Shutdown();
		glfwMakeContextCurrent(m_data->m_windowHandle);
#endif

		glfwDestroyWindow(m_data->m_windowHandle);
		m_data.reset();
	}

	bool RenderingSystem::CloseRequested()
	{
		return glfwWindowShouldClose(m_data->m_windowHandle) == 1;
	}

	void RenderingSystem::SubmitFrame()
	{
		auto& toExecute = m_data->GetCurrentCommandBuffer();
		toExecute.finish();

#if TE_MULTI_THREADED
		m_data->m_renderingThread.WaitAndExecute(&toExecute);
#else
		m_data->m_renderingContext.ExecuteCommandBuffer(&toExecute);
		glfwSwapBuffers(m_data->m_windowHandle);
#endif

		// Swap working buffer
		m_data->m_currentCommandBuffer = 1 - m_data->m_currentCommandBuffer;
		m_data->GetCurrentCommandBuffer().start();
	}

	void RenderingSystem::PollEvents()
	{
		glfwPollEvents();

		// Update input-state
		memcpy(m_data->m_oldKeyState, m_data->m_keyState, sizeof(m_data->m_keyState));

		for (int i = 0; i < static_cast<int>(Key::LAST_KEY); ++i)
		{
			m_data->m_keyState[i] = IsKeyDown(static_cast<Key>(i));
		}
	}

	double RenderingSystem::GetTime()
	{
		return glfwGetTime();
	}

	glm::vec2 RenderingSystem::GetCursorPosition()
	{
		double x, y;
		glfwGetCursorPos(m_data->m_windowHandle, &x, &y);
		return glm::vec2(x, y);
	}

	bool RenderingSystem::IsKeyDown(Key k)
	{
		return glfwGetKey(m_data->m_windowHandle, toGLFW(k)) == GLFW_PRESS;
	}

	bool RenderingSystem::IsMouseButtonDown(MouseButton key)
	{
		return glfwGetMouseButton(m_data->m_windowHandle, key == MouseButton::Right ? GLFW_MOUSE_BUTTON_RIGHT : GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	}

	void RenderingSystem::SetWindowTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_data->m_windowHandle, title.c_str());
	}

	bool RenderingSystem::WasPressed(Key key)
	{
		return m_data->m_keyState[static_cast<int>(key)] && !m_data->m_oldKeyState[static_cast<int>(key)];
	}

	void RenderingSystem::ClearScreen(const Graphics::ClearState& clearState)
	{
		ClearScreenData data;
		data.clearState = clearState;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::ClearScreen);
		cmdBuff.write(data);
	}

	ShaderProgramHandle RenderingSystem::CreateShaderProgram(const Graphics::ShaderInfo& si)
	{
		assert(m_data->m_numShaderPrograms < std::numeric_limits<decltype(m_data->m_numShaderPrograms)>::max());

		CreateShaderProgramData data;
		data.handle = { ++m_data->m_numShaderPrograms };
		data.siPtr = new Graphics::ShaderInfo(si);
		
		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::CreateShaderProgram);
		cmdBuff.write(data);

		return data.handle;
	}

	BufferHandle RenderingSystem::CreateBuffer()
	{
		assert(m_data->m_numBuffers < std::numeric_limits<decltype(m_data->m_numBuffers)>::max());

		CreateBufferData data;
		data.handle = { ++m_data->m_numBuffers };

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::CreateBuffer);
		cmdBuff.write(data);

		return data.handle;
	}

	void RenderingSystem::UpdateBuffer(BufferHandle buffer, void* bufferData, uint32_t size, BufferType usage)
	{
		void* dataCopy = nullptr;

		if (bufferData && size)
		{
			// TODO Allocator for frame instead of individual mallocs
			dataCopy = malloc(size);
			assert(dataCopy);
			memcpy(dataCopy, bufferData, size);
		}

		UpdateBufferData data;
		data.buffer = buffer;
		data.data = dataCopy;
		data.size = size;
		data.usage = usage;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::UpdateBuffer);
		cmdBuff.write(data);
	}

	Texture2DHandle RenderingSystem::CreateTexture2D()
	{
		assert(m_data->m_numTexture2D < std::numeric_limits<decltype(m_data->m_numTexture2D)>::max());

		CreateTexture2DData data;
		data.handle = { ++m_data->m_numTexture2D };

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::CreateTexture2D);
		cmdBuff.write(data);

		return data.handle;
	}

	void RenderingSystem::UpdateTexture2D(Texture2DHandle buffer, void* textureData, uint16_t width, uint16_t height, TextureType type)
	{
		void* dataCopy = nullptr;

		if (textureData)
		{
			size_t size = width * height * 4;
			// TODO Allocator for frame instead of individual mallocs
			dataCopy = malloc(size);
			assert(dataCopy && "dataCopy UpdateTexture2D");
			memcpy(dataCopy, textureData, size);
		}

		UploadTexture2DData data;
		data.buffer = buffer;
		data.data = dataCopy;
		data.height = height;
		data.width = width;
		data.type = type;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::UploadTexture2D);
		cmdBuff.write(data);
	}

	void RenderingSystem::SetCursorEnabled(bool enabled)
	{
		glfwSetInputMode(m_data->m_windowHandle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void RenderingSystem::BindTexture2D(uint8_t unit, Texture2DHandle texture)
	{
		BindTexture2DData data;
		data.texture = texture;
		data.unit = unit;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::BindTexture2D);
		cmdBuff.write(data);
	}

	void RenderingSystem::UseShaderProgram(ShaderProgramHandle handle)
	{
		UseShaderProgramData data;
		data.handle = handle;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::UseShaderProgram);
		cmdBuff.write(data);
	}

	void RenderingSystem::Draw(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t elements)
	{
		DrawData data;
		data.vertexBuffer = vertexBuffer;
		data.indexBuffer = indexBuffer;
		data.elements = elements;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::Draw);
		cmdBuff.write(data);
	}

	void RenderingSystem::BindUniformBuffer(uint8_t bindingIndex, BufferHandle buffer)
	{
		BindUniformBufferData data;
		data.bindingIndex = bindingIndex;
		data.buffer = buffer;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::BindUniformBuffer);
		cmdBuff.write(data);
	}

	void RenderingSystem::ReloadShaders()
	{
		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::ReloadShaders);
	}

	RenderTargetHandle RenderingSystem::CreateRenderTarget(RenderTargetOptions options)
	{
		assert(m_data->m_numRenderTargets < std::numeric_limits<decltype(m_data->m_numRenderTargets)>::max());

		CreateRenderTargetData data;
		data.handle = { ++m_data->m_numRenderTargets };
		data.options = options;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::CreateRenderTarget);
		cmdBuff.write(data);

		return data.handle;
	}

	void RenderingSystem::BindRenderTarget(RenderTargetHandle handle)
	{
		BindRenderTargetData data;
		data.handle = handle;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::BindRenderTarget);
		cmdBuff.write(data);
	}

	void RenderingSystem::BindRenderTargetTexture(uint8_t unit, RenderTargetHandle handle, RenderTargetTexture texture)
	{
		BindRenderTargetTexturesData data;
		data.unit = unit;
		data.texture = texture;
		data.handle = handle;

		auto& cmdBuff = m_data->GetCurrentCommandBuffer();
		cmdBuff.write(CommandBuffer::Command::BindRenderTargetTextures);
		cmdBuff.write(data);
	}

}

static void APIENTRY OGLDebugFunc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	(void)length;
	(void)id;
	(void)userParam;

	std::string srcName;
	switch (source)
	{
	case GL_DEBUG_SOURCE_API_ARB: srcName = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: srcName = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: srcName = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: srcName = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB: srcName = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER_ARB: srcName = "Other"; break;
	}

	std::string errorType;
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR_ARB: errorType = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: errorType = "Deprecated Functionality"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: errorType = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB: errorType = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB: errorType = "Performance"; break;
	case GL_DEBUG_TYPE_OTHER_ARB: errorType = "Other"; break;
	}

	std::string typeSeverity;
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH_ARB: typeSeverity = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB: typeSeverity = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW_ARB: typeSeverity = "Low"; break;
	}

	printf("\n--------- START OPENGL DEBUG OUTPUT ---------\n%s from %s,\t%s priority\nMessage: %s\n--------- END OPENGL DEBUG OUTPUT ---------\n\n",
		errorType.c_str(), srcName.c_str(), typeSeverity.c_str(), message);
}

static void GLFWErrorCallback(int error, const char* message)
{
	(void)error;
	printf("GLFW reported error: %s\n", message);
}
