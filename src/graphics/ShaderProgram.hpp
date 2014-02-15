#pragma once
#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "OpenGL.h"
#include "ShaderInfo.h"
#include "glm/fwd.hpp"

namespace Graphics
{
	class ShaderProgram
	{
	public:
		ShaderProgram();
		virtual ~ShaderProgram();

		// Noncopyable
		ShaderProgram(const ShaderProgram& other) = delete;
		ShaderProgram& operator=(const ShaderProgram& other) = delete;

		bool Load(const ShaderInfo& shaderInfo);
		void DeleteProgram();

		int GetAttribLocation(const std::string &s);

		static int GetUniformLocation(GLuint program, const std::string& name);
		static bool UpdateUniform(int programId, int location, const glm::vec4& m);
		static bool UpdateUniform(int programId, int location, const glm::vec3& m);
		static bool UpdateUniform(int programId, int location, const glm::mat4& m);
		static bool UpdateUniform(int programId, int location, float f);

		int GetUniformLocation(const std::string& name);
		bool UpdateUniform(const std::string&, const glm::mat4&);
		bool UpdateUniform(const std::string&, const glm::vec2&);
		bool UpdateUniform(const std::string&, const glm::vec3&);
		bool UpdateUniform(const std::string&, const glm::vec4&);
		bool UpdateUniform(const std::string&, float);
		bool UpdateUniformi(const std::string&, int);

		int  GetProgram() const;
		void UseProgram() const;

		bool Reload();
		bool IsLoaded() const;

	private:
		// Disallows automatic type conversions (since 
		// there are different glUniform*-functions for ints, floats, etc.)
		// (Alternativly could add suffixes to methods)
		template<typename T> void UpdateUniform(std::string, T arg);
		template<typename T> void UpdateUniform(int programId, int location, T arg);

		std::unordered_map<std::string, GLint> m_uniformLocations;

		unsigned int m_programId = 0;
		ShaderInfo   m_shaderInfo;
		bool         m_loadedFromFile = false;
		bool         m_successfullyLoaded = false;
	};

}
#endif // SHADERPROGRAM_HPP
