#pragma once

#include <vector>
#include "CG_Data.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>


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

		const size_t AddData(void* _Data) {
			eData.push_back(_Data);
			return eData.size() - 1;
		}

		void* GetData(int index) {
			return eData[index];
		}

		const std::vector<void*> GeteDataList() const { return this->eData; };

		void UpdateUniforms() const;
		glm::mat4 TransformMatrix;

	private:
		glm::vec4 Position{ 0, 0, 0, 1 };
		glm::vec3 Forward{ 0, 0, 1 }, Up{ 0, 1, 0 }, Right{ 1, 0, 0 };
		glm::vec3 Scale{ 1, 1, 1 };
		glm::quat Orientation;
		std::vector<CG_Data::Uniform*> EntityUniforms;
		
		std::vector<void*> eData;
		void UpdateMatrix();
		bool MatrixNeedsUpdating{ true };
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
		private:
			glm::vec3 eRelativePosition;
			glm::quat eRelativeOrientation;
			glm::mat4 LocalMatrix, GlobalMatrix;
		};

		class HJoint {
		public:
			HJoint(glm::vec3 _RelativePosition, glm::quat _Orientation = glm::quat(1, 0, 0, 0));
			void InitialiseJoint(glm::mat4 parent);
			void AddChild(HJoint *_Child);
			void AddNode(HNode *_Node);
			void Translate(glm::vec3 _Translation);
			void rotate(float _Degrees, glm::vec3 axis);
			void YawBy(float _Degrees);
			void RollBy(float _Degrees);
			void PitchBy(float _Degrees);
			std::vector<HJoint*> *GetChilder();
			
		private:
			glm::vec3 JointRelativePosition;
			glm::quat NodeOrientation;
			glm::mat4 LocalMatrix, GlobalMatrix, ParentMatrix, TranslationMatrix, RotationMatrix;
			std::vector<HNode*> nodes;
			std::vector<HJoint*> childer;
			void UpdateJoint(glm::mat4 _ParentMatrix);
		};

		Hierarchy();
		void InitialiseHierarchy();
		void SetRoot(HJoint *root);
		HJoint *GetRoot() const;

	private:
		HJoint* root;
	};

}