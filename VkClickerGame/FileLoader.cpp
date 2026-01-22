#include "pch.h"
#include "FileLoader.h"
#include <fstream>

namespace mem
{
	mem::Arr<char> loadFile(ARENA arena, const char* path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		assert(file.is_open());

		// Dump contents in the buffer.
		const size_t fileSize = file.tellg();
		const auto buffer = mem::Arr<char>(arena, fileSize);

		file.seekg(0);
		file.read(buffer.ptr(), static_cast<std::streamsize>(fileSize));
		file.close();

		return buffer;
	}
}