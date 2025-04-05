#include "lupch.h"
#include "Shader.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include <fstream>

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Static methods
	////////////////////////////////////////////////////////////////////////////////////
	std::string Shader::ReadGLSL(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		LU_ASSERT((file.is_open() && file.good()), std::format("Failed to open file: '{0}'", path.string()));

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		if (content.empty())
			LU_LOG_WARN("[Shader] GLSL file: '{0}' is empty, this will 'cause internal errors.", path.string());

		return content;
	}

	std::vector<char> Shader::ReadSPIRV(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios_base::ate | std::ios_base::binary);
		LU_ASSERT((file.is_open() && file.good()), std::format("Failed to open file: '{0}'", path.string()));

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		return buffer;
	}


}