#pragma once

#include <vector>
#include "Common.h"
#include "CG_Data.h"
namespace GL_Engine{
	class Shader
	{
	public:

		Shader();
		~Shader();
		
		const GLuint GetShaderID() const;

		//Compile a shader stage from a source text file. Returns Shader ID
		const uint8_t CompileShader();

		//Register a shader file to the pipeline
		bool RegisterShaderStageFromFile(const char *_FilePath, GLenum _StageType);

		//Register a shader source to the pipeline
		void RegisterShaderStage(const char* _ShaderSource, GLenum _StageType);

		//Register a shader attribute, to be bound at _Location
		void RegisterAttribute(const char* _AttributeName, GLuint _Location);

		//Activate the program
		void UseShader() const;

		//Register a Uniform.
		//Returns pointer to UBO (Object finalised after call to compile)
		CG_Data::Uniform* RegisterUniform(const char* _UniformName);


	private:
		struct ShaderStage {
			const char* Source;
			GLenum Type;
			GLuint ID;
		};
		struct Attribute {
			const char* AttributeName;
			GLuint Location;
		};
		struct UniformStruct{
			const char *Name;
			CG_Data::Uniform *UniformObject;
		};
		std::vector<ShaderStage*> shaderStages;
		std::vector<Attribute*> Attributes;
		std::vector<UniformStruct*> UBOs;

		GLuint ShaderID;
		bool initialised{ false };
		const GLuint CompileShaderStage(ShaderStage *stage);
	};

}