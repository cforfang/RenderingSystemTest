#include "ShaderProgram.hpp"
#include "OpenGL.h"

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ShaderUtils
{
	struct Status
	{
		std::string error;
		bool success;
	};

	// For InfoLogHelper below
#ifdef _MSC_VER
	typedef void(__stdcall *glGetXiv)(GLuint obj, GLenum type, GLint* param);
	typedef void(__stdcall *glGetXInfoLog)(GLuint obj, GLsizei maxLen, GLsizei * len, GLchar *infoLog);
#else
	typedef void(*glGetXiv)(GLuint obj, GLenum type, GLint* param);
	typedef void(*glGetXInfoLog)(GLuint obj, GLsizei maxLen, GLsizei * len, GLchar *infoLog);
#endif

	// Avoids duplication of infolog-fetching code
	// Example usage: 
	// const std::string& infoLog    = InfoLogHelper( glGetShaderiv,  glGetShaderInfoLog,  shaderObject );
	// const std::string& strInfoLog = InfoLogHelper( glGetProgramiv, glGetProgramInfoLog, program);
	const std::string InfoLogHelper(glGetXiv glGetXiv, glGetXInfoLog glGetXInfoLog, GLint object)
	{
		GLint infoLogLength;
		glGetXiv(object, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::unique_ptr<GLchar[]> strInfoLog(new GLchar[infoLogLength + 1]);
		glGetXInfoLog(object, infoLogLength, NULL, strInfoLog.get());

		return std::string(strInfoLog.get());
	}

	Status CompileShader(GLuint shader, const std::string &shaderText)
	{
		const char *strFileData = shaderText.c_str();
		glShaderSource(shader, 1, &strFileData, NULL);

		glCompileShader(shader);

		GLint compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE)
		{
			Status status;
			status.success = false;
			status.error = InfoLogHelper(glGetShaderiv, glGetShaderInfoLog, shader);
			return status;
		}

		Status status;
		status.success = true;
		return status;
	}

}

static std::string LoadFile(const std::string& file, const std::string& includeDir)
{
	std::ifstream in(file);

	if (!in.is_open())
	{
		std::cerr << "Couldn't load source from file " << file << std::endl;
		return "";
	}

	std::stringstream buffer;

	while (in.good())
	{
		std::string line;
		std::getline(in, line);

		if (line.length() > 1 && line.at(0) == '@')
		{
			std::string includeFile = includeDir + line.substr(1, line.find_first_of(' '));

			std::ifstream inc(includeFile);

			if (!inc.is_open())
			{
				std::cerr << "Couldn't include file '" << includeFile << "' while parsing " << file << std::endl;
				//throw std::runtime_error("Couldn't include file '" + includeFile + "' while parsing " + file);
			}
			else
			{
				std::string includeString;
				std::getline(inc, includeString, '\0');
				buffer << includeString << "\n";
			}
			inc.close();
		}
		else
		{
			buffer << line << "\n";
		}
	}

	in.close();

	return buffer.str();
}

namespace Graphics
{

	ShaderProgram::ShaderProgram()
		: m_programId(0), m_loadedFromFile(false)
	{

	}

	ShaderProgram::~ShaderProgram()
	{
		DeleteProgram();
	}

	int ShaderProgram::GetProgram() const
	{
		return m_programId;
	}

	bool ShaderProgram::Reload()
	{
		if (m_loadedFromFile)
		{
			// Invalidate uniform-location cache
			m_uniformLocations.clear();

			// Reload from files
			return Load(m_shaderInfo);		
		}

		return true;
	}

	bool ShaderProgram::Load(const ShaderInfo& shaderInfo)
	{
		std::string includeDir = shaderInfo.includeDir;
		m_shaderInfo = shaderInfo;
		m_loadedFromFile = true;
		m_successfullyLoaded = false;

		struct Shader
		{
			Shader() : handle(0), type(0) {};
			std::string file; // Nice when reporting errors
			std::string source;
			GLuint handle;
			GLenum type;
		};

		std::vector<Shader> shaders;

		if (shaderInfo.vsFile != "")
		{
			Shader s;
			s.file = shaderInfo.vsFile;
			s.source = LoadFile(s.file, includeDir);
			s.type = GL_VERTEX_SHADER;
			shaders.push_back(s);
		}

		if (shaderInfo.tcFile != "")
		{
			Shader s;
			s.file = shaderInfo.tcFile;
			s.source = LoadFile(s.file, includeDir);
			s.type = GL_TESS_CONTROL_SHADER;
			shaders.push_back(s);
		}

		if (shaderInfo.teFile != "")
		{
			Shader s;
			s.file = shaderInfo.teFile;
			s.source = LoadFile(s.file, includeDir);
			s.type = GL_TESS_EVALUATION_SHADER;
			shaders.push_back(s);
		}

		if (shaderInfo.gsFile != "")
		{
			Shader s;
			s.file = shaderInfo.gsFile;
			s.source = LoadFile(s.file, includeDir);
			s.type = GL_GEOMETRY_SHADER;
			shaders.push_back(s);
		}

		if (shaderInfo.fsFile != "")
		{
			Shader s;
			s.file = shaderInfo.fsFile;
			s.source = LoadFile(s.file, includeDir);
			s.type = GL_FRAGMENT_SHADER;
			shaders.push_back(s);
		}

		if (m_programId == 0)
		{
			// Create program if necessary
			m_programId = glCreateProgram();
		}

		// Create, attach, and compile
		bool success = true;
		std::string error;
		for (Shader& s : shaders)
		{
			s.handle = glCreateShader(s.type);
			glAttachShader(m_programId, s.handle);

			ShaderUtils::Status status = ShaderUtils::CompileShader(s.handle, s.source);
			if (status.success == false)
			{
				// Continue so it reports compile-errors for all shaders
				success = false;
				error += "(" + s.file + ") ";
				error += status.error;
				continue;
			}
		}

		// Mark shaders for deletion (done on detach)
		for (Shader& s : shaders)
		{
			glDeleteShader(s.handle);
		}

		// Compilation failure
		if (!success)
		{
			// Detach shaders
			for (Shader& s : shaders)
			{
				glDetachShader(m_programId, s.handle);
			}

			std::cerr << error << std::endl;
			return false;
		}

		// Link
		glLinkProgram(m_programId);

		// Detach all shaders
		for (Shader& s : shaders)
		{
			glDetachShader(m_programId, s.handle);
		}

		// Check link-status
		GLint linkStatus;
		glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);

		if (linkStatus == GL_FALSE)
		{
			const std::string& strInfoLog = ShaderUtils::InfoLogHelper(glGetProgramiv, glGetProgramInfoLog, m_programId);
			std::cerr << strInfoLog << std::endl;
			return false;
		}

		m_successfullyLoaded = true;

		return true;
	}

	bool ShaderProgram::IsLoaded() const
	{
		return m_successfullyLoaded;
	}

	bool ShaderProgram::UpdateUniform(int programId, int location, float f)
	{
		glProgramUniform1f(programId, location, f);
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(int programId, int location, const glm::mat4& m)
	{
		if (location == -1) return false;
		glProgramUniformMatrix4fv(programId, location, 1, GL_FALSE, glm::value_ptr(m));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(const std::string& name, const glm::mat4& m)
	{
		GLint location = GetUniformLocation(name);
		if (location == -1) return false;
		return UpdateUniform(m_programId, location, m);
	}

	bool ShaderProgram::UpdateUniform(const std::string& name, const glm::vec2& v)
	{
		GLint location = GetUniformLocation(name);
		if (location == -1) return false;
		glProgramUniform2fv(m_programId, location, 1, glm::value_ptr(v));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(const std::string& name, const glm::vec3& v)
	{
		GLint location = GetUniformLocation(name);
		if (location == -1) return false;
		glProgramUniform3fv(m_programId, location, 1, glm::value_ptr(v));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(const std::string& name, const glm::vec4& v)
	{
		GLint location = GetUniformLocation(name);
		if (location > -1)
			glProgramUniform4fv(m_programId, location, 1, glm::value_ptr(v));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(int programId, int location, const glm::vec4& v)
	{
		glProgramUniform4fv(programId, location, 1, glm::value_ptr(v));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(int programId, int location, const glm::vec3& v)
	{
		glProgramUniform3fv(programId, location, 1, glm::value_ptr(v));
		return location > -1;
	}

	bool ShaderProgram::UpdateUniform(const std::string& name, float f)
	{
		GLint location = GetUniformLocation(name);
		if (location == -1) return false;
		glProgramUniform1f(m_programId, location, f);
		return location > -1;
	}

	bool ShaderProgram::UpdateUniformi(const std::string& name, int i)
	{
		GLint location = GetUniformLocation(name);
		if (location == -1) return false;
		glProgramUniform1i(m_programId, location, i);
		return location > -1;
	}

	void ShaderProgram::UseProgram() const
	{
		glUseProgram(m_programId);
	}

	int ShaderProgram::GetAttribLocation(const std::string &name)
	{
		GLint location = glGetAttribLocation(m_programId, name.c_str());
		return location;
	}

	int ShaderProgram::GetUniformLocation(GLuint program, const std::string& name)
	{
		return glGetUniformLocation(program, name.c_str());
	}

	int ShaderProgram::GetUniformLocation(const std::string& name)
	{
		const auto& f = m_uniformLocations.find(name);
		if (f != m_uniformLocations.end())
		{
			return f->second;
		}

		GLint location = glGetUniformLocation(m_programId, name.c_str());
		m_uniformLocations[name] = location;

		return location;
	}

	void ShaderProgram::DeleteProgram()
	{
		if (m_programId != 0)
		{
			glDeleteProgram(m_programId);
			m_programId = 0;
		}
	}

}