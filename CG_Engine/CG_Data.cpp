#include "CG_Data.h"
#include <stdexcept>
#include <glm/vec3.hpp>

namespace GL_Engine{
	namespace CG_Data{

#pragma region VBO
		VBO::VBO(){
			glGenBuffers(1, &this->ID);
			initialised = true;
			Target = GL_ARRAY_BUFFER;
		}
		VBO::VBO(void* _Data, uint64_t _DataSize, GLenum _Usage, GLenum _Target){
			glGenBuffers(1, &this->ID);
			Target = _Target;
			Usage = _Usage;
			SetVBOData(_Data, _DataSize);
			initialised = true;
		}

		VBO::~VBO(){
			if (initialised) {
				glDeleteBuffers(1, &this->ID);
				initialised = false;
			}
		}

		const GLuint VBO::GetID() const{
			return this->ID;
		}
		void VBO::BindVBO() const{
			glBindBuffer(Target, this->ID);
		}

		void VBO::SetVBOData(void* _Data, uint64_t _DataSize) const{
			glBindBuffer(Target, this->ID);
			glBufferData(Target, _DataSize, _Data, Usage);
		}

#pragma region VAO

		VAO::VAO(){
			glGenVertexArrays(1, &this->VAOId);
			initialised = true;
		}

		VAO::~VAO(){
			if (initialised) {
				this->BindVAO();
				for (auto &vbo : this->VBOs){
					vbo.reset();
				}
				VBOs.clear();
				glDeleteVertexArrays(1, &this->VAOId);
				initialised = false;
			}
		}

		void VAO::Cleanup(){
			if (initialised) {
				this->BindVAO();
				for (auto &vbo : this->VBOs){
					vbo.reset();
				}
				VBOs.clear();
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

#pragma region ModelAttribute
		ModelAttribute::ModelAttribute() {

		}
		ModelAttribute::~ModelAttribute(){
			
		}
		ModelAttribute::ModelAttribute(const aiScene *_Scene, unsigned int index) {
			this->BindVAO();
			MeshIndex = TexCoordIndex = NormalIndex = IndicesIndex = -1;
			auto mesh = _Scene->mMeshes[index];
			std::vector<unsigned int> indices;
			for (int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
				}
			}
			std::unique_ptr<VBO> indexVBO = std::make_unique<VBO>(&indices[0], indices.size() * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
			this->VBOs.push_back(std::move(indexVBO));
			this->IndicesIndex = 0;
			this->VertexCount = indices.size();
			glEnableVertexAttribArray(2);

			std::unique_ptr<VBO> meshVBO = std::make_unique<VBO>(mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
			this->VBOs.push_back(std::move(meshVBO));
			glEnableVertexAttribArray(0);
			MeshIndex = this->VBOs.size() - 1;
			if (mesh->HasNormals()) {
				std::unique_ptr<VBO> normalVBO = std::make_unique<VBO>(mesh->mNormals, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
				this->VBOs.push_back(std::move(normalVBO));
				NormalIndex = this->VBOs.size() - 1;
				glEnableVertexAttribArray(1);
			}
	//		if (mesh->HasTextureCoords()) {
	//			std::unique_ptr<VBO> texCoordVBO = std::make_unique<VBO>(new VBO(&mesh->mTextureCoords[0], mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW));
	//			this->VBOs.push_back(texCoordVBO);
	//			TexCoordIndex = this->VBOs.size() - 1;
	//		}
		}

		VBO* ModelAttribute::GetVBO(int index) {
			return this->VBOs[index].get();
		}

		const uint64_t ModelAttribute::GetVertexCount() const {
			return this->VertexCount;
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

#pragma region UBO
		GLuint UBO::UBO_Count = 0;

#pragma region ModelLoader

		const aiScene* ModelLoader::LoadModel(std::string &_Path, unsigned int _Flags) {
			const aiScene* scene = aImporter.ReadFile(_Path, _Flags);
			if (!scene) {
				throw std::runtime_error("Error loading model " + _Path + "\n" + aImporter.GetErrorString() + "\n");
			}

			return scene;
		}

		ModelAttribList ModelLoader::LoadScene(const aiScene *_Scene) {
			auto numMeshes = _Scene->mNumMeshes;
			ModelAttribList attributes;
			attributes.reserve(numMeshes);

			for (auto i = 0; i < _Scene->mNumMeshes; i++) {
				auto m = _Scene->mMeshes[i];
				std::shared_ptr<ModelAttribute> newAttrib = std::make_shared<ModelAttribute>(_Scene, i);
				attributes.push_back(std::move(newAttrib));
			}

			return attributes;
		}

	}
}