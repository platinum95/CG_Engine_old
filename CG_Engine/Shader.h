#pragma once

#include <vector>
#include "Common.h"

namespace GL_Engine{
	class Shader
	{
	public:

		Shader();
		~Shader();

		const GLuint GetShaderID() const;

		//Compile a shader stage from a source text file. Returns 0 if successful
		uint8_t AddShaderStageFromFile(const char *_FilePath, GLenum _StageType);

		//Compile a shader stage from a source text string. Returns 0 if successful
		uint8_t AddShaderStage(const char * _ShaderSource, GLenum _StageType);

		//Compile a shader stage from a source text file. Returns Shader ID
		const uint8_t CompileShader();

		void shader_test();

	private:
		std::vector<GLuint> shaderStageIDs;
		GLuint ShaderID;



	};

}