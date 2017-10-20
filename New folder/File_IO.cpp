#include "File_IO.h"
#include <fstream>

namespace GL_Engine{

	File_IO::File_IO()
	{
	}


	File_IO::~File_IO()
	{
	}


	const char* File_IO::LoadTextFile(const char* _FilePath, uint8_t *result){
		if (_FilePath == "" || _FilePath == nullptr){
			*result = 1;
			return nullptr;
		}
		std::ifstream FileStream(_FilePath, std::ios::in | std::ios::binary);
		if (!FileStream){
			*result = 2;
			return nullptr;
		}
		std::string *FileStr = new std::string();
		FileStream.seekg(0, std::ios::end);	//Jump to end of file stream
		FileStr->resize(FileStream.tellg());	//Get stream size and resize string accordingly
		FileStream.seekg(0, std::ios::beg);
		FileStream.read(&(*FileStr)[0], FileStr->size());
		FileStream.close();

		*result = 0;
		return(FileStr->c_str());	//Return a const char array rather than std string
	}

}