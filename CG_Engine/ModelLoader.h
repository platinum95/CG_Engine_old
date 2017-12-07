#pragma 
#include "Entity.h"
namespace GL_Engine {

	struct AttribNodePair {
		std::shared_ptr<ModelAttribute> Attribute;
		Hierarchy::HNode *Node;
	};

	/*-------------ModelLoader Class------------*/
	/*
	*Handles the loading and memory of models from file
	*/
	
	class ModelLoader {
	public:
		ModelAttribList LoadModel(std::string &_PathBase, std::string&_ModelFile, unsigned int _Flags);
		std::pair<std::unique_ptr<Hierarchy>, std::vector<AttribNodePair>> LoadHierarchyModel(std::string &_PathBase, std::string &_ModelFile, unsigned int _Flags);
		std::unique_ptr<RiggedModel> LoadRiggedModel(std::string &_PathBase, std::string &_ModelFile, unsigned int _Flags);
		void CleanUp();
		static std::vector<std::shared_ptr<CG_Data::Texture>> LoadMaterial(const aiMaterial *material, const aiTextureType _Type, std::string &_PathBase,
			std::vector<std::shared_ptr<CG_Data::Texture>> &_Textures);
		static std::shared_ptr<CG_Data::Texture> LoadTexture(std::string& _Path, GLuint _Unit);

	private:
		Assimp::Importer aImporter;
		static std::map <std::string, std::shared_ptr<CG_Data::Texture>> CachedTextures;
	};
	

}