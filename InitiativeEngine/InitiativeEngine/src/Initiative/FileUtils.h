#pragma once
#include "Core.h"
#include "Log.h"

namespace itv
{
	std::vector<char> readFile(const std::string& filename)
	{
		std::filesystem::path cwd = std::filesystem::current_path();
		ITV_LOG("Curent Directory is {0} ", cwd.string());

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}





}


