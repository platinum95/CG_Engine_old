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

	class Bone {
	public:
		Bone(const aiBone *_aiBone, std::shared_ptr<Bone> _ParentBone, std::shared_ptr<Skeleton> _ParentSkeleton) {
			this->Name = _aiBone->mName.data;
			this->OffsetMatrix = AiToGLMMat4(_aiBone->mOffsetMatrix);
			this->ParentBone = _ParentBone;
			this->ParentSkeleton = _ParentSkeleton;
		}
		
		void SetNode(aiNode *_Node) { this->BoneNode = _Node; }
		void SetAnimationNode(aiNodeAnim* _AnimNode) { this->AnimationNode = _AnimNode; }
		const std::string &GetName() const { return this->Name; }
		const glm::mat4 &GetTransformation() const { return this->UpstreamTransformations; };
	protected:
		glm::mat4 UpstreamTransformations;
		glm::mat4 OffsetMatrix;
		
	private:
		std::string Name;
		aiNode *BoneNode;
		aiNodeAnim *AnimationNode;
		std::shared_ptr<Bone> ParentBone;
		std::vector<std::shared_ptr<Bone>> ChildBones;
		std::shared_ptr<Skeleton> ParentSkeleton;
	};

	class Skeleton {
	public:

	protected:

	private:
		
	};

	class RiggedModel {
	public:
		RiggedModel();
		~RiggedModel();
		std::unique_ptr<RenderPass> GenerateRenderpass();
	protected:
		Entity ModelEntity;
		Skeleton ModelRig;

	private:
		static void RiggedModelRenderPass(RenderPass &_Pass, void* _Data);
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