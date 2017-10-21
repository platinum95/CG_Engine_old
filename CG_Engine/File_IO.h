#pragma once

#include <stdint.h>
#include <memory>

namespace GL_Engine{
	class File_IO
	{
	public:
		File_IO();
		~File_IO();
		static const char* LoadTextFile(const char* _FilePath, uint8_t *result);
	};
}

