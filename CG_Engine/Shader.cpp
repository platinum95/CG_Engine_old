#include "Shader.h"
#include "File_IO.h"
#include <stdexcept>

namespace GL_Engine{

	Shader::Shader(){
	}


	Shader::~Shader(){
		glDeleteProgram(this->ShaderID);
	}

	const GLuint Shader::GetShaderID() const {
		return ShaderID;
	}


	uint8_t Shader::AddShaderStageFromFile(const char *_FilePath, GLenum _StageType){
		uint8_t FileLoadResult;
		const char* ShaderText = File_IO::LoadTextFile(_FilePath, &FileLoadResult);
		if (FileLoadResult || ShaderText == nullptr){
			throw std::runtime_error("Error loading shader file " + std::string(_FilePath));
		}

		return AddShaderStage(ShaderText, _StageType);
	}

	uint8_t Shader::AddShaderStage(const char * _ShaderSource, GLenum _StageType){
		GLuint StageID = glCreateShader(_StageType);

		if (StageID == 0) {
			throw std::runtime_error("Error creating shader stage!");
			return 1;
		}
		//Bind the source to the Stage, and compile
		glShaderSource(StageID, 1, (const GLchar**)&_ShaderSource, NULL);
		glCompileShader(StageID);
		GLint Result;
		// check for shader related errors using glGetShaderiv
		glGetShaderiv(StageID, GL_COMPILE_STATUS, &Result);
		if (!Result) {
			GLchar ErrorBuffer[1024];
			glGetShaderInfoLog(StageID, 1024, NULL, ErrorBuffer);
			throw std::runtime_error("Error compiling shader!\n" + std::string(ErrorBuffer));
			return 1;
		}

		this->shaderStageIDs.push_back(StageID);
		return 0;
	}

	const uint8_t Shader::CompileShader(){
		this->ShaderID = glCreateProgram();
		if (!ShaderID) {
			throw std::runtime_error("Error generating Shader ID!");
			return 0;
		}

		// Attach each compiled stage to the Shader Program
		for (auto StageID : this->shaderStageIDs){
			glAttachShader(ShaderID, StageID);
		}

		glLinkProgram(ShaderID);
		GLchar ErrorBuffer[1024] = { 0 };
		GLint Result;
		glGetProgramiv(ShaderID, GL_LINK_STATUS, &Result);
		if (!Result) {
			glGetProgramInfoLog(ShaderID, sizeof(ErrorBuffer), NULL, ErrorBuffer);
			throw std::runtime_error("Error linking Shader!\n" + std::string(ErrorBuffer));
			return 0;
		}

		// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
		glValidateProgram(ShaderID);
		// check for program related errors using glGetProgramiv
		glGetProgramiv(ShaderID, GL_VALIDATE_STATUS, &Result);
		if (!Result) {
			glGetProgramInfoLog(ShaderID, sizeof(ErrorBuffer), NULL, ErrorBuffer);
			throw std::runtime_error("Error linking Shader!\n" + std::string(ErrorBuffer));
			return 0;
		}

		for (auto StageID : this->shaderStageIDs){
			glDeleteShader(StageID);	//Stages no longer needed, so clean them up
		}
		return ShaderID;
	}

	void Shader::shader_test(){
		GLuint shaderProgramID = glCreateProgram();
		if (shaderProgramID == 0) {
			fprintf(stderr, "Error creating shader program\n");
			exit(1);
		}

		// Create two shader objects, one for the vertex, and one for the fragment shader
		AddShaderStageFromFile("v.glsl", GL_VERTEX_SHADER);
		glAttachShader(shaderProgramID, this->shaderStageIDs[0]);
		AddShaderStageFromFile("f.glsl", GL_FRAGMENT_SHADER);
		glAttachShader(shaderProgramID, this->shaderStageIDs[1]);

		GLint Success = 0;
		GLchar ErrorLog[1024] = { 0 };


		// After compiling all shader objects and attaching them to the program, we can finally link it
		glLinkProgram(shaderProgramID);
		// check for program related errors using glGetProgramiv
		glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
		if (Success == 0) {
			glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
			exit(1);
		}

		// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
		glValidateProgram(shaderProgramID);
		// check for program related errors using glGetProgramiv
		glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
		if (!Success) {
			glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
			fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
			exit(1);
		}
		// Finally, use the linked shader program
		// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
		glUseProgram(shaderProgramID);
		this->ShaderID = shaderProgramID;
	}


}