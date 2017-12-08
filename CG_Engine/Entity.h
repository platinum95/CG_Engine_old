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
		return tmp;
	}
	class MeshNode {
	public:
		MeshNode(const aiNode* _Node, std::shared_ptr<MeshNode> _ParentNode) {
			this->TranformationMatrix = AiToGLMMat4(_Node->mTransformation);
			this->Name = _Node->mName.data;
			this->ParentNode = _ParentNode;
		}

		void SetParent(std::shared_ptr<MeshNode> _ParentNode) {
			this->ParentNode = _ParentNode;
		}
		const glm::mat4 GetTransformationMatrix() const {
			return this->TranformationMatrix;
		}
		std::shared_ptr<MeshNode> ParentNode;
	protected:
	private:
		glm::mat4 TranformationMatrix;
		std::string Name;
		
	};

	class Bone {
	public:
		Bone(const aiBone *_aiBone, std::shared_ptr<MeshNode> _Node) {
			this->Name = _aiBone->mName.data;
			this->OffsetMatrix = AiToGLMMat4(_aiBone->mOffsetMatrix);
			this->BoneNode = _Node;
		}

//		void SetParents(std::shared_ptr<Bone> _ParentBone) {
//			this->ParentBone = _ParentBone;
//			this->BoneNode->SetParent(_ParentBone->BoneNode);
//		}
		void SetTransformation(glm::mat4 _GlobalInverseTransform) {
			this->Transformation = _GlobalInverseTransform * UpstreamTransformations * BoneNode->GetTransformationMatrix() * this->OffsetMatrix;
		}
		void SetNode(std::shared_ptr<MeshNode> _Node) { this->BoneNode = _Node; }
		void SetAnimationNode(aiNodeAnim* _AnimNode) { this->AnimationNode = _AnimNode; }
		const std::string &GetName() const { return this->Name; }
		const glm::mat4 &GetTransformation() const { return this->UpstreamTransformations; };
	protected:
		glm::mat4 UpstreamTransformations;
		glm::mat4 OffsetMatrix, Transformation;

	private:
		std::string Name;
		std::shared_ptr<MeshNode> BoneNode;
		aiNodeAnim *AnimationNode;
//		std::shared_ptr<Bone> ParentBone;
		std::vector<std::shared_ptr<Bone>> ChildBones;

		glm::mat4& GetUpstreamMatrix() {

			auto n = BoneNode;		//maybe parent here
			std::vector<glm::mat4> mats;

			while (n != nullptr){
				glm::mat4 tmp_mat = n->GetTransformationMatrix();
				mats.push_back(tmp_mat);
				n = n->ParentNode;
			}

			glm::mat4 concatenated_transforms(1.0f);
			for (auto mat = mats.rbegin(); mat != mats.rend(); mat++) {
				concatenated_transforms *= *mat;
			}
	//		for (int i = mats.size() - 1; i >= 0; i--)
	//			concatenated_transforms *= mats.at(i);

			return concatenated_transforms;
		}
	};

	class Skeleton {
	public:
		Skeleton(std::map<std::string, std::shared_ptr<Bone>> &&BoneList) { this->Bones = std::move(BoneList); }
		std::map<std::string, std::shared_ptr<Bone>> Bones;

		void Update() {
			for (auto bone : Bones) {
				bone.second->SetTransformation(this->GlobalInverseMatrix);
			}
		}
		glm::mat4 GlobalInverseMatrix;
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
		std::vector<std::shared_ptr<Bone>> Bones;
		std::map<std::string, unsigned int> BoneIndex;
	private:
		uint64_t VertexCount = 0;
	};
	using ModelAttribList = std::vector<std::shared_ptr<ModelAttribute>>;

	//Shader can take max 5 bones. Weight is 0 if bone not used
	class RiggedModel : public Entity {
	public:
		RiggedModel(std::unique_ptr<Skeleton> _Rig, aiMatrix4x4 &_GlobalInverseMatrix, ModelAttribList &&_AttributeList) {
			this->ModelRig = std::move(_Rig);
			this->GlobalInverseTransform = AiToGLMMat4(_GlobalInverseMatrix);
			this->ModelAttributes = std::forward<ModelAttribList>(_AttributeList);
			this->ModelRig->GlobalInverseMatrix = this->GlobalInverseTransform;
			Orientation = glm::quat(1, 0, 0, 0);
			eData.push_back(glm::value_ptr(this->TransformMatrix));
		}
		~RiggedModel() {};
		std::unique_ptr<RenderPass> GenerateRenderpass(Shader* _Shader) {
			std::unique_ptr<RenderPass> renderPass = std::make_unique<RenderPass>();
			renderPass->renderFunction = RiggedModelRenderer;
			renderPass->shader = _Shader;
			renderPass->Data = (void*)this;
			return std::move(renderPass);
		}
		Skeleton *GetRig() const { return this->ModelRig.get(); }
	protected:
		Entity ModelEntity;
		std::unique_ptr<Skeleton> ModelRig;
		glm::mat4 GlobalInverseTransform;

	private:
		static void RiggedModelRenderer(RenderPass &_Pass, void* _Data);
		ModelAttribList ModelAttributes;
	};


	class Hierarchy {
	public:
		class HNode : public Entity {
		public:
			HNode();
			glm::mat4 UpdateMatrix(glm::mat4 _JointMatrix);
			glm::mat4* GetGlobalMatrix();
			void SetPosition(const glm::vec3 _Pos);
			void SetPosition(const float x, const float y, const float z);
			glm::mat4 LocalMatrix, GlobalMatrix;
		private:
			glm::vec3 eRelativePosition;
			glm::quat eRelativeOrientation;
			//glm::mat4 LocalMatrix, GlobalMatrix;
		};

		class HJoint {
		public:
			HJoint(glm::vec3 _RelativePosition, glm::quat _Orientation = glm::quat(1, 0, 0, 0));
			HJoint(glm::mat4 _LocalMatrix);
			void InitialiseJoint(glm::mat4 parent);
			void AddChild(HJoint *_Child);
			void AddNode(HNode *_Node);
			void Translate(glm::vec3 _Translation);
			void rotate(float _Degrees, glm::vec3 axis);
			void YawBy(float _Degrees);
			void RollBy(float _Degrees);
			void PitchBy(float _Degrees);
			std::vector<HJoint*> *GetChilder();
			void UpdateJoint(glm::mat4 _ParentMatrix);

		private:
			glm::vec3 JointRelativePosition;
			glm::quat NodeOrientation;
			glm::mat4 LocalMatrix, GlobalMatrix, ParentMatrix, TranslationMatrix, RotationMatrix;
			std::vector<HNode*> nodes;
			std::vector<HJoint*> childer;
			
		};

		Hierarchy();
		void InitialiseHierarchy();
		void SetRoot(HJoint *root);
		HJoint *GetRoot() const;
		HJoint* GetJoint(std::string _Name) { return JointMap[_Name]; }
		void AddJoint(std::string _Name, HJoint* _Joint) { JointMap[_Name] = _Joint; }
		void Update() { root->UpdateJoint(glm::mat4(1.0)); }
		std::map<std::string, HJoint*> JointMap;

	private:
		HJoint* root;
		
	};

}