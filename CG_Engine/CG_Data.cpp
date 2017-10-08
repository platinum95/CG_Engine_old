#include "CG_Data.h"


namespace GL_Engine{
	namespace CG_Data{

#pragma region VBO
		VBO::VBO(){
			glGenBuffers(1, &this->VBOId);
			initialised = true;
		}
		VBO::VBO(void* _Data, uint64_t _DataSize, GLenum _Usage){
			glGenBuffers(1, &this->VBOId);
			SetVBOData(_Data, _DataSize, _Usage);
			initialised = true;
		}

		VBO::~VBO(){
			if (initialised) {
				glDeleteBuffers(1, &this->VBOId);
				initialised = false;
			}
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
			initialised = true;
		}

		VAO::~VAO(){
			if (initialised) {
				glDeleteVertexArrays(1, &this->VAOId);
				initialised = false;
			}
		}

		const GLuint VAO::GetID() const{
			return this->VAOId;
		}

		void VAO::BindVAO() const{
			glBindVertexArray(this->VAOId);
		}

#pragma region Uniform
		Uniform::Uniform(size_t _DataSize) {
	
		}
		Uniform::Uniform() {

		}
		Uniform::~Uniform() {

		}
		const GLuint Uniform::GetID() const {
			return this->ID;
		}
		void Uniform::BindUniform() const {
			
		}

		void Uniform::SetID(GLint _ID) {
			this->ID = _ID;
		}

		void Uniform::setData(void* _Data) {
			this->NeedsUpdating = true;
			this->Data = _Data;
		}

	}
}