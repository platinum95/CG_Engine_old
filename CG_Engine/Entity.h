#pragma once

#include <vector>
#include "CG_Data.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <map>
#include "Shader.h"

namespace GL_Engine {
	class Entity {
	public:
		Entity();
		~Entity();

		void SetPosition(glm::vec3 _Position);
		void Translate(glm::vec3 _Translation);
		void YawBy(float _Degrees);
		void PitchBy(float _degrees);
		void RollBy(float _Degrees);
		void RotateBy(float _Degrees, glm::vec3 _Axis);
		void SetScale(glm::vec3 _Scale);
		void ScaleBy(glm::vec3 _ScaleBy);
		void SetOrientation(glm::quat _Orientation){ this->Orientation = _Orientation; this->MatrixNeedsUpdating = true; }
		void Rotate(glm::quat _Rotation) { 
			this->Orientation = _Rotation * Orientation;
			this->Forward = _Rotation * Forward;
			this->Up = _Rotation * Up;
			this->Right = _Rotation * Right;
			this->MatrixNeedsUpdating = true;
		}

		const glm::mat4 GetTransformMatrix();
		const glm::quat GetOrientation() const;
		const glm::vec4 GetPosition() const { return this->Position; }

		const glm::mat4 TransformBy(glm::mat4 _Transform){
			return this->TransformMatrix = _Transform * this->TransformMatrix;
		}

		const uint16_t AddData(void* _Data) {
			eData.push_back(_Data);
			return (uint16_t) eData.size() - 1;
		}

		void SetData(int index, void* data) {
			eData[index] = data;
		}

		void SetActive(bool _State) {
			this->Active = _State;
		}

		void* GetData(int index) {
			return eData[index];
		}

		bool isActive() const { return Active; }
		void Activate() { Active = true; }
		void Deactivate() { Active = false; }

		const std::vector<void*> GeteDataList() const { return this->eData; };

		void UpdateUniforms() const;
		glm::mat4 TransformMatrix;

	protected:
		glm::vec4 Position{ 0, 0, 0, 1 };
		glm::vec3 Forward{ 0, 0, 1 }, Up{ 0, 1, 0 }, Right{ 1, 0, 0 };
		glm::vec3 Scale{ 1, 1, 1 };
		glm::quat Orientation;
		std::vector<CG_Data::Uniform*> EntityUniforms;
		bool Active{ true };
		std::vector<void*> eData;
		void UpdateMatrix();
		bool MatrixNeedsUpdating{ true };
	};

	struct BatchUnit {
		Entity* entity;
		bool active{ true };
	};
	struct eDataUniLink {
		CG_Data::Uniform *uniform;
		uint16_t eDataIndex;
	};
	struct RenderPass {
		~RenderPass();
		BatchUnit* AddBatchUnit(Entity* _Entity);
		void SetDrawFunction(std::function<void(void)> _dFunc);
		void AddDataLink(CG_Data::Uniform *_Uniform, uint16_t _DataIndex) {
			eDataUniLink link = { _Uniform, _DataIndex };
			this->dataLink.push_back(link);
		}

		std::vector<eDataUniLink> dataLink;
		void* Data;
		Shader* shader;
		std::vector<std::unique_ptr<BatchUnit>> batchUnits;
		std::shared_ptr<CG_Data::VAO> BatchVao;
		std::function<void(RenderPass&, void*)> renderFunction;
		std::function<void(void)> DrawFunction;
		std::vector<std::shared_ptr<CG_Data::Texture>> Textures;
	};



	static glm::mat4 AiToGLMMat4(const aiMatrix4x4& in_mat){
		glm::mat4 tmp;
		tmp[0][0] = in_mat.a1;	tmp[1][0] = in_mat.b1;
		tmp[2][0] = in_mat.c1;	tmp[3][0] = in_mat.d1;
		tmp[0][1] = in_mat.a2;	tmp[1][1] = in_mat.b2;
		tmp[2][1] = in_mat.c2;	tmp[3][1] = in_mat.d2;
		tmp[0][2] = in_mat.a3;	tmp[1][2] = in_mat.b3;
		tmp[2][2] = in_mat.c3;	tmp[3][2] = in_mat.d3;
		tmp[0][3] = in_mat.a4;	tmp[1][3] = in_mat.b4;
		tmp[2][3] = in_mat.c4;	tmp[3][3] = in_mat.d4;
		return glm::transpose(tmp);
	}


	class MeshBone {
	public:
		MeshBone(const aiBone* _Bone);
		const glm::mat4 &GetFinalTransform(const glm::mat4 &_GlobalInverse, const glm::mat4& nodeGlobalTransform);
		glm::mat4 OffsetMatrix;
		glm::mat4 FinalTransformation;
		std::string Name;
	};

	class SceneBone {
	public:
		SceneBone(const aiBone *_Bone);
		void AddMeshBone(std::shared_ptr<MeshBone> _mBone);
		void UpdateBone(const glm::mat4 &GlobalInverse, const glm::mat4 &_GlobalTransform);
		glm::mat4 GlobalTransformation{ 1.0f };
		std::string Name;
		std::vector<std::shared_ptr<MeshBone>> meshBones;
	};

	class NodeAnimation {
	public:
		std::string Name;
		std::vector<std::pair<glm::vec3, double>> Positions, Scalings;
		std::vector < std::pair < glm::quat, double>> Rotations;
		double AnimationLength;
		NodeAnimation(const aiNodeAnim *animNode, double _Length);
	};

	class SceneNode {
		public:
			SceneNode(const aiNode* _node);
			void Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse, unsigned int AnimationID, double Time);
			void AddChild(std::shared_ptr<SceneNode> _node);
			void Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse);
			std::shared_ptr<SceneBone> sceneBone;
			std::vector<std::shared_ptr<SceneNode>> ChildNodes;
			std::shared_ptr<NodeAnimation> Animation{ nullptr };
			glm::mat4 NodeTransform, GlobalTransform;
			std::string Name;
	private:
		static glm::mat4 GetInterpolatedScale(std::vector<std::pair<glm::vec3, double>> Scalings, double time);
		static glm::mat4 GetInterpolatedTranslate(std::vector<std::pair<glm::vec3, double>> Translations, double time);
		static glm::mat4 GetInterpolatedRotate(std::vector<std::pair<glm::quat, double>> Rotations, double time);
	};

	class Skeleton {
	public:
		Skeleton(std::shared_ptr<SceneNode> _Root, std::map<std::string, std::shared_ptr<SceneNode>> SkeletonNodeMap);
		std::shared_ptr<SceneNode> rootNode;
		void Update();
		void Update(unsigned int AnimationID, double Time);
		glm::mat4 GlobalInverseMatrix;
		std::map<std::string, std::shared_ptr<SceneNode>> NodeMap;
	protected:
		
	private:
		
	};

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
		std::vector<std::string> BoneNames;
		std::vector<std::shared_ptr<MeshBone>> meshBones;
		std::map<std::string, unsigned int> BoneIndex;
	private:
		uint64_t VertexCount = 0;
	};
	using ModelAttribList = std::vector<std::shared_ptr<ModelAttribute>>;

	//Shader can take max 5 bones. Weight is 0 if bone not used
	class RiggedModel : public Entity {
	public:
		RiggedModel(std::unique_ptr<Skeleton> _Rig, ModelAttribList &&_AttributeList);
		~RiggedModel();
		std::unique_ptr<RenderPass> GenerateRenderpass(Shader* _Shader);
		void Update();
		void Update(unsigned int AnimationID, double Time);
		Skeleton *GetRig() const;
	protected:
		Entity ModelEntity;
		std::unique_ptr<Skeleton> ModelRig;
		glm::mat4 GlobalInverseTransform;

	private:
		static void RiggedModelRenderer(RenderPass &_Pass, void* _Data);
		ModelAttribList ModelAttributes;
	};


}