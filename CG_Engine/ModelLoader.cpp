#include "ModelLoader.h"

namespace GL_Engine {
	using namespace CG_Data;

#pragma region ModelAttribute
	ModelAttribute::ModelAttribute() {

	}
	ModelAttribute::~ModelAttribute() {

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
		while (i == 0) {//mesh->mTextureCoords[i]){
			std::vector<float> texCoords;
			texCoords.reserve(mesh->mNumVertices * sizeof(float) * 2);

			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				texCoords.push_back(mesh->mTextureCoords[i][j].x);
				texCoords.push_back(mesh->mTextureCoords[i][j].y);
			}
			std::unique_ptr<VBO> texCoordVBO = std::make_unique<VBO>(&texCoords[0], sizeof(float) * texCoords.size(), GL_STATIC_DRAW);
			texCoordVBO->BindVBO();
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
			this->VBOs.push_back(std::move(texCoordVBO));
			TexCoordIndex = (int) this->VBOs.size() - 1;
			texCoords.clear();
			i++;
		}
		if (mesh->HasTangentsAndBitangents()) {
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
		if (mesh->mMaterialIndex != -1) {
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

	glm::mat4 aMatToGMat(const aiMatrix4x4 &m)
	{
		glm::mat4 gMat;

		gMat[0][0] = m.a1;
		gMat[0][1] = m.b1;
		gMat[0][2] = m.c1;
		gMat[0][3] = m.d1;
		gMat[1][0] = m.a2;
		gMat[1][1] = m.b2;
		gMat[1][2] = m.c2;
		gMat[1][3] = m.d2;
		gMat[2][0] = m.a3;
		gMat[2][1] = m.b3;
		gMat[2][2] = m.c3;
		gMat[2][3] = m.d3;
		gMat[3][0] = m.a4;
		gMat[3][1] = m.b4;
		gMat[3][2] = m.c4;
		gMat[3][3] = m.d4;

		return gMat;
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
	void LoadNode(const aiScene* Scene, aiNode* Node, Hierarchy *hierarchy, std::string &_PathBase) {
		aiMatrix4x4 nodeTransformation = Node->mTransformation;
		auto glmTransformation = aMatToGMat(nodeTransformation);
		glm::vec4 NodePos(0.0f, 0.0f, 0.0f, 1.0f);
		NodePos = glmTransformation * NodePos;
		auto joint = new Hierarchy::HJoint(glm::vec3(NodePos));
		for (unsigned int i = 0; i < Node->mNumMeshes; ++i) {
			std::shared_ptr<ModelAttribute> newAttrib = std::make_shared<ModelAttribute>(Scene, Node->mMeshes[i], _PathBase);
			
		}
		
	}

	std::unique_ptr<Hierarchy> ModelLoader::LoadHierarchyModel(std::string &_PathBase, std::string & _ModelFile, unsigned int _Flags) {
		auto hierarchy = std::make_unique<Hierarchy>();
		const aiScene* _Scene = aImporter.ReadFile(_PathBase + _ModelFile, _Flags);
		if (!_Scene) {
			throw std::runtime_error("Error loading model " + _PathBase + _ModelFile + "\n" + aImporter.GetErrorString() + "\n");
		}
		auto rootNode = _Scene->mRootNode;
		LoadNode(_Scene, rootNode, hierarchy.get(), _PathBase);

	}

	void ModelLoader::CleanUp() {
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


}