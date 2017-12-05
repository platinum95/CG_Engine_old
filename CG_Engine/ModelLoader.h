#pragma 
#include "Entity.h"
namespace GL_Engine {

	/*-------------ModelAttribute Class------------*/
	/*
	*Handles the data loaded in from a model file.
	*/
	class ModelAttribute : public CG_Data::VAO {
	public:
		ModelAttribute();
		~ModelAttribute();
		ModelAttribute(const aiScene *_Scene, unsigned int index, std::string &_PathBase);

		CG_Data::VBO* GetVBO(int index);
		int MeshIndex, NormalIndex, TexCoordIndex, IndicesIndex;
		const uint64_t GetVertexCount() const;
		void AddTexture(std::shared_ptr<CG_Data::Texture> _Texture) {
			this->ModelTextures.push_back(_Texture);
		}
		std::vector<std::shared_ptr<CG_Data::Texture>> ModelTextures;
	private:
		uint64_t VertexCount = 0;
	};

	/*-------------ModelLoader Class------------*/
	/*
	*Handles the loading and memory of models from file
	*/
	using ModelAttribList = std::vector<std::shared_ptr<ModelAttribute>>;
	class ModelLoader {
	public:
		ModelAttribList LoadModel(std::string &_PathBase, std::string&_ModelFile, unsigned int _Flags);
		std::unique_ptr<Hierarchy> LoadHierarchyModel(std::string &Path, std::string &_ModelFile, unsigned int _Flags);
		void CleanUp();
		static std::vector<std::shared_ptr<CG_Data::Texture>> LoadMaterial(const aiMaterial *material, const aiTextureType _Type, std::string &_PathBase,
			std::vector<std::shared_ptr<CG_Data::Texture>> &_Textures);
		static std::shared_ptr<CG_Data::Texture> LoadTexture(std::string& _Path, GLuint _Unit);

	private:
		Assimp::Importer aImporter;
		static std::map <std::string, std::shared_ptr<CG_Data::Texture>> CachedTextures;
	};

}