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

#pragma region Texture
		Texture::Texture(void* _Data, GLint width, GLint height, GLuint _Unit, GLuint _ImageFormat, std::function<void()> _Parameters, GLenum _Target) {
			glGenTextures(1, &this->ID);
			this->Target = _Target;
			this->Unit = _Unit;
			glActiveTexture(this->Unit);
			glBindTexture(this->Target, this->ID);
			_Parameters();

			glTexImage2D(this->Target, 0, GL_RGBA, width, height, 0, _ImageFormat, GL_UNSIGNED_BYTE, _Data);
			glGenerateMipmap(this->Target);
		}
		Texture::Texture(GLuint _Unit, GLenum _Target) {
			glGenTextures(1, &this->ID);
			this->Target = _Target;
			this->Unit = _Unit;
		}

		void Texture::SetUnit(const GLuint _Unit) {
			this->Unit = _Unit;
		}

		void Texture::Bind() {
			glActiveTexture(this->Unit);
			glBindTexture(this->Target, this->ID);
		}

		const GLuint Texture::GetID() const { return this->ID; }

#pragma region ModelAttribute
		ModelAttribute::ModelAttribute() {

		}
		ModelAttribute::~ModelAttribute(){
			
		}
		ModelAttribute::ModelAttribute(const aiScene *_Scene, unsigned int index, std::string& _PathBase) {
			//0 - Vertices
			//1 - Texture coords
			//2 - Normals
			//3 - Tangents
			//4 - Bitangents
			this->BindVAO();
			MeshIndex = TexCoordIndex = NormalIndex = IndicesIndex = -1;
			auto mesh = _Scene->mMeshes[index];
			std::vector<unsigned int> indices;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
				}
			}
			
			std::unique_ptr<VBO> indexVBO = std::make_unique<VBO>(&indices[0], indices.size() * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
			this->VBOs.push_back(std::move(indexVBO));
			this->IndicesIndex = 0;
			this->VertexCount = indices.size();
			indices.clear();

			std::unique_ptr<VBO> meshVBO = std::make_unique<VBO>(mesh->mVertices, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
			meshVBO->BindVBO();
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(0);
			this->VBOs.push_back(std::move(meshVBO));
			MeshIndex = (int) this->VBOs.size() - 1;

			if (mesh->HasNormals()) {
				std::unique_ptr<VBO> normalVBO = std::make_unique<VBO>(mesh->mNormals, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
				normalVBO->BindVBO();
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
				glEnableVertexAttribArray(2);
				this->VBOs.push_back(std::move(normalVBO));
				NormalIndex = (int) this->VBOs.size() - 1;
			}
			int i = 0;
			while(i==0){//mesh->mTextureCoords[i]){
				std::vector<float> texCoords;
				texCoords.reserve(mesh->mNumVertices * sizeof(float) * 2);

				for(unsigned int j = 0; j < mesh->mNumVertices; j++){
					texCoords.push_back(mesh->mTextureCoords[i][j].x);
					texCoords.push_back(mesh->mTextureCoords[i][j].y);
				}
				std::unique_ptr<VBO> texCoordVBO = std::make_unique<VBO>(&texCoords[0], sizeof(float) * texCoords.size() , GL_STATIC_DRAW);
				texCoordVBO->BindVBO();
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
				glEnableVertexAttribArray(1);
				this->VBOs.push_back(std::move(texCoordVBO));
				TexCoordIndex = (int) this->VBOs.size() - 1;
				texCoords.clear();
				i++;
			}
			if(mesh->HasTangentsAndBitangents()){
				std::unique_ptr<VBO> tangeantVBO = std::make_unique<VBO>(mesh->mTangents, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
				std::unique_ptr<VBO> bitangeantVBO = std::make_unique<VBO>(mesh->mBitangents, mesh->mNumVertices * sizeof(aiVector3D), GL_STATIC_DRAW);
				tangeantVBO->BindVBO();
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
				glEnableVertexAttribArray(3);
				bitangeantVBO->BindVBO();
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
				glEnableVertexAttribArray(4);
				
				this->VBOs.push_back(std::move(tangeantVBO));
				this->VBOs.push_back(std::move(bitangeantVBO));
			}
			if(mesh->mMaterialIndex != -1){
				aiMaterial *material = _Scene->mMaterials[mesh->mMaterialIndex];
				ModelLoader::LoadMaterial(material,
					aiTextureType_DIFFUSE, _PathBase, this->ModelTextures);
				ModelLoader::LoadMaterial(material,
					aiTextureType_NORMALS, _PathBase, this->ModelTextures);
				ModelLoader::LoadMaterial(material,
					aiTextureType_HEIGHT, _PathBase, this->ModelTextures);
				ModelLoader::LoadMaterial(material,
					aiTextureType_SPECULAR, _PathBase, this->ModelTextures);

			}
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
		UBO::UBO() {};
		UBO::UBO(void* _Data, size_t _DataSize) {
			this->Data = _Data;
			this->DataSize = _DataSize;
			this->Target = GL_UNIFORM_BUFFER;
			this->Usage = GL_DYNAMIC_DRAW;
			this->SetVBOData(Data, DataSize);
			this->BindingPost = UBO_Count++;
			glBindBufferBase(GL_UNIFORM_BUFFER, this->BindingPost, this->ID);
		}
		void UBO::UpdateUBO() const {
			glBindBuffer(GL_UNIFORM_BUFFER, this->ID);
			GLvoid* UBO_Pointer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
			memcpy(UBO_Pointer, this->Data, this->DataSize);
			glUnmapBuffer(GL_UNIFORM_BUFFER);
		}
		const GLuint UBO::GetBindingPost()const {
			return this->BindingPost;
		}

#pragma region ModelLoader

		std::map <std::string, std::shared_ptr<Texture>> ModelLoader::CachedTextures;

		ModelAttribList ModelLoader::LoadModel(std::string &_PathBase, std::string&_ModelFile, unsigned int _Flags) {
			const aiScene* _Scene = aImporter.ReadFile(_PathBase + _ModelFile, _Flags);
			if (!_Scene) {
				throw std::runtime_error("Error loading model " + _PathBase + _ModelFile + "\n" + aImporter.GetErrorString() + "\n");
			}

			auto numMeshes = _Scene->mNumMeshes;
			ModelAttribList attributes;
			attributes.reserve(numMeshes);

			for (unsigned int i = 0; i < _Scene->mNumMeshes; i++) {
				auto m = _Scene->mMeshes[i];
				std::shared_ptr<ModelAttribute> newAttrib = std::make_shared<ModelAttribute>(_Scene, i, _PathBase);
				attributes.push_back(std::move(newAttrib));
			}
			aImporter.FreeScene();
			return attributes;
		}
		void ModelLoader::CleanUp(){
			aImporter.FreeScene();
			CachedTextures.clear();
		}
		std::vector<std::shared_ptr<Texture>> ModelLoader::LoadMaterial(const aiMaterial *material, const aiTextureType _Type, std::string &_PathBase,
			std::vector<std::shared_ptr<Texture>> &_Textures) {
			for (unsigned int i = 0; i < material->GetTextureCount(_Type); i++) {
				aiString str;
				material->GetTexture(_Type, i, &str);
				std::string std_str = std::string(str.C_Str());
				if (std_str.size() == 0)
					continue;
				std_str = _PathBase + std_str;
				if (CachedTextures[std_str]) {
					_Textures.push_back(CachedTextures[std_str]);
					continue;
				}
				const auto texture = LoadTexture(std_str, GL_TEXTURE0 + (GLuint)_Textures.size());
				_Textures.push_back(texture);
				CachedTextures[std_str] = texture;
			}
			return _Textures;
		}

		std::shared_ptr<Texture> ModelLoader::LoadTexture(std::string& _Path, GLuint _Unit) {
			int width, height, nChannels;
			void* data = File_IO::LoadImageFile(_Path, width, height, nChannels, true);
			GLint format = nChannels == 3 ? GL_RGB : GL_RGBA;
			auto parameters = []() {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			};
			std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(data, width, height, _Unit, format, parameters, GL_TEXTURE_2D);
			free(data);
			//File_IO::FreeImageData(data);
			return newTexture;
		}


#pragma region FBO
			FBO::RenderbufferObject::RenderbufferObject(uint16_t _Width, uint16_t _Height, GLenum _Type) {
				glGenRenderbuffers(1, &this->ID);
				glBindRenderbuffer(GL_RENDERBUFFER, this->ID);
				glRenderbufferStorage(GL_RENDERBUFFER, _Type, _Width, _Height);

			}
			void FBO::RenderbufferObject::Bind() const {

			}

			FBO::TexturebufferObject::TexturebufferObject(uint16_t _Width, uint16_t _Height, uint8_t _Unit) {
				auto parameters = []() {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				};
				TextureObject = std::make_shared<Texture>(nullptr, _Width, _Height, GL_TEXTURE0 + _Unit, GL_RGB, parameters, GL_TEXTURE_2D);
				this->ID = TextureObject->GetID();
			}
			void FBO::TexturebufferObject::TexturebufferObject::Bind() const {

			}
			const std::shared_ptr<Texture> FBO::TexturebufferObject::GetTexture() const { 
				return this->TextureObject; 
			}
		
	
			FBO::FBO() {
				glGenFramebuffers(1, &this->ID);
				Initialised = true;
			}
			FBO::~FBO() {
				if (Initialised) {
					this->CleanUp();
					Initialised = false;
				}
			}
			void FBO::CleanUp() {
				if (Initialised) {
					glDeleteFramebuffers(1, &this->ID);
					Initialised = false;
				}
			}
			std::shared_ptr<FBO::AttachmentBufferObject> FBO::AddAttachment(AttachmentType _Attachment, uint16_t _Width, uint16_t _Height) {
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				switch (_Attachment) {
				case TextureAttachment: {
					std::shared_ptr<TexturebufferObject> TexObj = std::make_shared<TexturebufferObject>(_Width, _Height, TextureAttachmentCount);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + TextureAttachmentCount++, GL_TEXTURE_2D, TexObj->ID, 0);
					this->Attachments.push_back(std::move(TexObj));
					break;
				}
				case StencilAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_STENCIL_INDEX8);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
				}
				case DepthAttachment: {
					std::shared_ptr<RenderbufferObject> rbo = std::make_shared<RenderbufferObject>(_Width, _Height, GL_DEPTH_COMPONENT32);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo->ID);
					this->Attachments.push_back(std::move(rbo));
					break;
				}
				}
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
					this->complete = true;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				std::shared_ptr<AttachmentBufferObject> abo = Attachments.back();
				return abo;

			}
			void FBO::Bind(uint8_t _ColourAttachment) const {
				if (!this->complete)
					throw std::runtime_error("Attempting to bind incomplete framebuffer!\n");
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffer(GL_COLOR_ATTACHMENT0 + _ColourAttachment);
				glViewport(0, 0, 1280, 720);
			}

			void FBO::Bind(uint16_t _Count, const GLenum* _ColourAttachments) const {
				if (!this->complete)
					throw std::runtime_error("Attempting to bind incomplete framebuffer!\n");
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, this->ID);
				glDrawBuffers(_Count, _ColourAttachments);
				glViewport(0, 0, 1280, 720);
			}
			const GLuint FBO::GetID() const {
				return this->ID;
			}
			void FBO::Unbind() const {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

	}
}