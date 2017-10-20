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
		Uniform::Uniform(GLint _Location, void* _Data, std::function<void(const CG_Data::Uniform&)> _Callback){
			this->ID = _Location;
			this->Data = _Data;
			this->UpdateCallback = _Callback;
			this->Initialised = true;
		}
		Uniform::Uniform() {
			this->ID = -1;
			this->Data = nullptr;
			this->UpdateCallback = nullptr;
			this->Initialised = false;
		}
		Uniform::~Uniform() {

		}
		const GLint Uniform::GetID() const {
			return this->ID;
		}

		void Uniform::SetID(GLint _ID) {
			this->ID = _ID;
		}

		void Uniform::SetData(const void* _Data) {
			this->NeedsUpdating = true;
			this->Data = _Data;
		}

		const void* Uniform::GetData() const {
			return this->Data; 
		}

		void Uniform::Update() const{
			if (!Initialised)
				return;
			UpdateCallback(*this);
		}
		void Uniform::SetUpdateCallback(std::function<void(const CG_Data::Uniform &u)> _callback){
			this->UpdateCallback = _callback;
			this->Initialised = true;
		}

	}
}