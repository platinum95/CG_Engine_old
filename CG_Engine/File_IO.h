#pragma once

#include <stdint.h>
#include <memory>
#include <string>


namespace GL_Engine{
	class File_IO
	{
	public:
		File_IO();
		~File_IO();
		static const char* LoadTextFile(const char* _FilePath, uint8_t *result);
	//	static void* LoadPNGImage(std::string _Path, int &width, int &height);
	//	static void* LoadRawImage(std::string _Path, int &width, int &height);
		static void* LoadImageFile(std::string _Path, int &width, int &height);
	};
}

