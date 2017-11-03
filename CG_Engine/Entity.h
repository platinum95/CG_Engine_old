#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include "CG_Data.h"
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
		void SetScale(glm::vec3 _Scale);
		void ScaleBy(glm::vec3 _ScaleBy);
		void SetOrientation(glm::quat _Orientation){ this->Orientation = _Orientation; this->MatrixNeedsUpdating = true; }

		const glm::mat4 GetTransformMatrix();
		const glm::quat GetOrientation() const;
		const glm::vec4 GetPosition() const { return this->Position; }

		const glm::mat4 TransformBy(glm::mat4 _Transform){
			return this->TransformMatrix = _Transform * this->TransformMatrix;
		}

		const uint16_t AddData(void* _Data) {
			eData.push_back(_Data);
			return eData.size() - 1;
		}

		void* GetData(int index) {
			return eData[index];
		}

		const std::vector<void*> GeteDataList() const { return this->eData; };

		void UpdateUniforms() const;
	private:
		glm::vec4 Position{ 0, 0, 0, 1 };
		glm::vec3 Forward{ 0, 0, 1 }, Up{ 0, 1, 0 }, Right{ 1, 0, 0 };
		glm::vec3 Scale{ 1, 1, 1 };
		glm::quat Orientation;
		std::vector<CG_Data::Uniform*> EntityUniforms;
		glm::mat4 TransformMatrix;
		std::vector<void*> eData;
		void UpdateMatrix();
		bool MatrixNeedsUpdating{ true };
	};

	class Hierarchy {
	public:
		struct HNode {
			Entity* entity{ nullptr };
			std::vector<HNode*> Childer;
			glm::vec3 NodePosition;
			void RotateBy(float _Degrees, glm::vec3 _Axis, glm::vec3 _Pos){
				glm::quat versor = glm::angleAxis(_Degrees, _Axis);
				NodePosition -= _Pos;
				NodePosition = versor * NodePosition;
				NodePosition += _Pos;

				glm::vec3 entityPos = entity->GetPosition();
				entityPos -= _Pos;
				entityPos = versor * entityPos;
				entityPos += _Pos;
				entity->SetPosition(entityPos);
				entity->SetOrientation(versor * entity->GetOrientation());

				for (auto c : Childer)
					c->RotateBy(_Degrees, _Axis, _Pos);
			}
			
			void RollBy(float _Degrees){
				RotateBy(_Degrees, glm::vec3(0, 0, 1), this->NodePosition);
			}
			void PitchBy(float _Degrees){
				RotateBy(_Degrees, glm::vec3(1, 0, 0), this->NodePosition);
			}
			void YawBy(float _Degrees){
				RotateBy(_Degrees, glm::vec3(0, 1, 0), this->NodePosition);
			}

			~HNode() {
				for (auto n : Childer) {
					if (!n)
						delete n;
				}
			}
		};

		Hierarchy() {
			root = nullptr;
		}
		~Hierarchy() {
			if (!root)
				delete root;
		}
		HNode *InitialiseHierarchy(Entity* _Entity, const glm::vec3 &_Pos) {
			root = new HNode;
			root->entity = _Entity;
			root->NodePosition = _Pos;
			root->Childer = std::vector<HNode*>();
			return root;
		}

		HNode *AddChild(HNode* _Node, Entity* entity, const glm::vec3 &_Pos) {
			HNode *newNode = new HNode;
			newNode->NodePosition = _Pos;
			newNode->entity = entity;
			_Node->Childer.push_back(newNode);
			return newNode;
		}

		HNode* GetRoot() const { return root; }


	private:
		HNode* root;
	};

}