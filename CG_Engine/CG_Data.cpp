#include "CG_Data.h"


namespace GL_Engine{
	namespace CG_Data{

#pragma region VBO
		VBO::VBO(){
			glGenBuffers(1, &this->VBOId);

		}
		VBO::VBO(void* _Data, uint64_t _DataSize, GLenum _Usage){
			glGenBuffers(1, &this->VBOId);
			SetVBOData(_Data, _DataSize, _Usage);
		}

		VBO::~VBO(){
			glDeleteBuffers(1, &this->VBOId);
		}

		const GLuint VBO::GetID() const{
			return this->VBOId;
		}
		void VBO::BindVBO() const{
			glBindBuffer(GL_ARRAY_BUFFER, this->VBOId);
		}

		void VBO::SetVBOData(void* _Data, uint64_t _DataSize, GLenum _Usage) const{
			glBindBuffer(GL_ARRAY_BUFFER, this->VBOId);
			glBufferData(GL_ARRAY_BUFFER, _DataSize, _Data, _Usage);
		}

#pragma region VAO

		VAO::VAO(){
			glGenVertexArrays(1, &this->VAOId);
		}

		VAO::~VAO(){
			glDeleteVertexArrays(1, &this->VAOId);
		}

		const GLuint VAO::GetID() const{
			return this->VAOId;
		}

		void VAO::BindVAO() const{
			glBindVertexArray(this->VAOId);
		}

	}
}