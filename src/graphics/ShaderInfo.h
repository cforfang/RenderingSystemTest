#pragma once

#include <string>

namespace Graphics
{
	struct ShaderInfo
	{
		static ShaderInfo VSFS(const std::string& vs, const std::string& fs)
		{
			ShaderInfo si;
			si.setVertexShaderFile(vs);
			si.setFragmentShaderFile(fs);
			return si;
		}

		static ShaderInfo VSFS(const std::string& vs, const std::string& fs, const std::string& includeDir)
		{
			ShaderInfo si;
			si.setVertexShaderFile(vs);
			si.setFragmentShaderFile(fs);
			si.setLookForIncludesDir(includeDir);
			return si;
		}

		void setVertexShaderFile(const std::string& str)
		{
			vsFile = str;
		}

		void setFragmentShaderFile(const std::string& str)
		{
			fsFile = str;
		}

		void setTesselationControlShaderFile(const std::string& str)
		{
			tcFile = str;
		}

		void setTesselationEvaulationShaderFile(const std::string& str)
		{
			teFile = str;
		}

		void setGeometryShaderFile(const std::string& str)
		{
			gsFile = str;
		}

		void setLookForIncludesDir(const std::string& str)
		{
			includeDir = str;
		}

		size_t GetHash() const
		{
			std::string str = vsFile + fsFile + tcFile + teFile + gsFile;
			return std::hash<std::string>()(str);
		}

		std::string vsFile{ "" };
		std::string tcFile{ "" };
		std::string teFile{ "" };
		std::string gsFile{ "" };
		std::string fsFile{ "" };
		std::string includeDir{ "" };
	};
}