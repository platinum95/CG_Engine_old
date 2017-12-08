#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace GL_Engine {
#pragma region ENTITY
	Entity::Entity() {
		Orientation = glm::quat(1, 0, 0, 0);
		eData.push_back(glm::value_ptr(this->TransformMatrix));
	}

	Entity::~Entity() {
	}


	void Entity::SetPosition(glm::vec3 _Position) {
		this->Position = glm::vec4(_Position, 1.0);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::Translate(glm::vec3 _Translation) {
		this->Position += glm::vec4(_Translation, 0.0);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::YawBy(float _Degrees) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Up);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Right = glm::rotate(Versor, this->Right);
		this->MatrixNeedsUpdating = true;
	}
	void Entity::PitchBy(float _degrees) {
		float Radians = glm::radians(_degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Right);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Up = glm::rotate(Versor, this->Up);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::RollBy(float _Degrees) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, this->Forward);

		Orientation = Versor * Orientation;
		this->Right = glm::rotate(Versor, this->Right);
		this->Up = glm::rotate(Versor, this->Up);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::RotateBy(float _Degrees, glm::vec3 _Axis) {
		float Radians = glm::radians(_Degrees);
		glm::quat Versor = glm::angleAxis(Radians, _Axis);

		Orientation = Versor * Orientation;
		this->Forward = glm::rotate(Versor, this->Forward);
		this->Right = glm::rotate(Versor, this->Right);
		this->MatrixNeedsUpdating = true;
	}

	void Entity::SetScale(glm::vec3 _Scale) {
		this->Scale = _Scale;
		MatrixNeedsUpdating = true;
	}

	void Entity::ScaleBy(glm::vec3 _Scale) {
		this->Scale *= _Scale;
		MatrixNeedsUpdating = true;
	}

	const glm::quat Entity::GetOrientation() const {
		return this->Orientation;
	}

	void Entity::UpdateUniforms() const {
		for (const auto uniform : this->EntityUniforms) {
			uniform->Update();
		}
	}



	const glm::mat4 Entity::GetTransformMatrix() {
		//Check if the local values have changed
		if (MatrixNeedsUpdating) {
			UpdateMatrix();	//Update matrix if so
			MatrixNeedsUpdating = false;
		}
		//Return the model matrix
		return this->TransformMatrix;
	}

	void Entity::UpdateMatrix() {
		//Generate rotation matrix from the orientation versor
		glm::mat4 R = glm::toMat4(Orientation);		

		//Create a translation matrix from identity
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(this->Position));	

		//Generate a scale matrix from identity
		glm::mat4 S = glm::scale(glm::mat4(1.0), this->Scale);

		//Regenerate local axes for next set of rotations
		this->Forward = glm::vec3(R * glm::vec4(0, 0, 1, 0));
		this->Up = glm::vec3(R * glm::vec4(0, 1, 0, 0));
		this->Right = glm::vec3(R * glm::vec4(1, 0, 0, 0));

		//Rotation matrix needs to be inverted to correctly
		//rotate the vertices. (I think, I was having 
		//problems when R isn't inverted)
		this->TransformMatrix = T * glm::inverse(R) * S;

	}
#pragma endregion

#pragma region HIERARCHY

	/*-----------------Hierarchy---------------*/

	Hierarchy::Hierarchy() {

	}
	void Hierarchy::InitialiseHierarchy() {
		this->root->InitialiseJoint(glm::mat4(1.0));
	}
	void Hierarchy::SetRoot(HJoint *_root) {
		this->root = _root;
	}

	Hierarchy::HJoint* Hierarchy::GetRoot() const {
		return this->root;
	}

	/*-----------------HNode---------------*/

	Hierarchy::HNode::HNode() {
		eRelativeOrientation = glm::quat(1, 0, 0, 0);
		eRelativePosition = glm::vec3(0, 0, 0);
	}

	void Hierarchy::HNode::SetPosition(const glm::vec3 _Pos) {
		this->eRelativePosition = _Pos;
	}
	void Hierarchy::HNode::SetPosition(const float x, const float y, const float z) {
		this->eRelativePosition = glm::vec3(x, y, z);
	}

	glm::mat4 Hierarchy::HNode::UpdateMatrix(glm::mat4 _JointMatrix) {
		LocalMatrix = glm::mat4(1.0);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->Scale);
		glm::mat4 R = glm::mat4(1.0);
		glm::mat4 T = glm::translate(glm::mat4(1.0), eRelativePosition);
		LocalMatrix = T * scale;
		GlobalMatrix = _JointMatrix;// *scale;
		return GlobalMatrix;
	}

	glm::mat4* Hierarchy::HNode::GetGlobalMatrix() {
		return &(this->GlobalMatrix);
	}


	/*-----------------HJoint---------------*/
	
	Hierarchy::HJoint::HJoint(glm::vec3 _RelativePosition, glm::quat _Orientation) {
		this->NodeOrientation = _Orientation;
		this->JointRelativePosition = _RelativePosition;

		this->TranslationMatrix = glm::translate(glm::mat4(1.0), this->JointRelativePosition);
		this->RotationMatrix = glm::inverse(glm::toMat4(this->NodeOrientation));

		this->LocalMatrix = this->TranslationMatrix * this->RotationMatrix;
	}

	Hierarchy::HJoint::HJoint(glm::mat4 _LocalMatrix) {
		this->LocalMatrix = _LocalMatrix;
		glm::decompose(_LocalMatrix, glm::vec3(), this->NodeOrientation, this->JointRelativePosition, glm::vec3(), glm::vec4());
		this->TranslationMatrix = glm::translate(glm::mat4(1.0), this->JointRelativePosition);
		this->RotationMatrix = glm::inverse(glm::toMat4(this->NodeOrientation));
	}

	void Hierarchy::HJoint::InitialiseJoint(glm::mat4 parent) {
		this->GlobalMatrix = parent * this->LocalMatrix;
		this->ParentMatrix = parent;

		for (auto a : nodes) {
			a->UpdateMatrix(this->GlobalMatrix);
		}
		for (auto c : childer) {
			c->InitialiseJoint(this->GlobalMatrix);
		}
	}


	void Hierarchy::HJoint::UpdateJoint(glm::mat4 _ParentMatrix) {
		this->GlobalMatrix = _ParentMatrix * this->LocalMatrix;
		this->ParentMatrix = _ParentMatrix;
		for (auto a : nodes) {
			a->UpdateMatrix(this->GlobalMatrix);
		}
		for (auto c : childer) {
			c->UpdateJoint(this->GlobalMatrix);
		}
	}

	void Hierarchy::HJoint::Translate(glm::vec3 _Translation) {
		this->JointRelativePosition += _Translation;
		this->TranslationMatrix = glm::translate(glm::mat4(1.0), this->JointRelativePosition);
		this->LocalMatrix = this->TranslationMatrix * this->RotationMatrix;

		UpdateJoint(ParentMatrix);
	}
	void Hierarchy::HJoint::rotate(float _Degrees, glm::vec3 axis) {
		glm::quat orient = glm::angleAxis(glm::radians(_Degrees), axis);
		NodeOrientation = orient * NodeOrientation;
		this->RotationMatrix = glm::inverse(glm::toMat4(this->NodeOrientation));
		this->LocalMatrix = this->TranslationMatrix * this->RotationMatrix;

		UpdateJoint(ParentMatrix);
	}
	void Hierarchy::HJoint::YawBy(float _Degrees) {
		rotate(_Degrees, glm::vec3(0, 1, 0));
	}
	void Hierarchy::HJoint::RollBy(float _Degrees) {
		rotate(_Degrees, glm::vec3(0, 0, 1));
	}
	void Hierarchy::HJoint::PitchBy(float _Degrees) {
		rotate(_Degrees, glm::vec3(1, 0, 0));
	}

	void Hierarchy::HJoint::AddChild(HJoint *_Child) {
		this->childer.push_back(_Child);
	}

	void Hierarchy::HJoint::AddNode(HNode *_Node) {
		this->nodes.push_back(_Node);
	}


	std::vector<Hierarchy::HJoint*> *Hierarchy::HJoint::GetChilder() {
		return &this->childer;
	}


	void sanityCheck(glm::mat4 &matrix) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (matrix[i][j] > 100 || matrix[i][j] < -100) {
					std::cout << "sanity failed, value " << matrix[i][j] << std::endl;
				}
			}
		}
	}

#pragma endregion

	void RiggedModel::RiggedModelRenderer(RenderPass& _Pass, void* _Data) {
		
		RiggedModel *Model = static_cast<RiggedModel*>(_Data);
		auto Rig = Model->GetRig();
		_Pass.shader->UseShader();

		for (auto l : _Pass.dataLink) {
			l.uniform->SetData(Model->GetData(l.eDataIndex));
			l.uniform->Update();
		}
		Model->UpdateUniforms();

		glm::mat4 t(1.0f);
		auto modelMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "model");
		glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(Model->GetTransformMatrix()));

		
		for (auto attrib : Model->ModelAttributes) {
			attrib->BindVAO();
			for (auto tex : attrib->ModelTextures) {
				tex->Bind();
			}
			auto boneMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "BoneMatrices");
			std::vector<glm::mat4> boneMatrices((const size_t) 55, glm::mat4(1.0));
			int i = 0;
			if (attrib->meshBones.size() > 0) {
				for (auto bone : attrib->meshBones) {
				//	boneMatrices.push_back(bone->FinalTransformation);
					sanityCheck(bone->FinalTransformation);
				}
				glUniformMatrix4fv(boneMatLoc, 55, GL_FALSE, glm::value_ptr(boneMatrices[0]));
			}
			else {
				glm::mat4 id(1.0);
				auto boneMatLoc = glGetUniformLocation(_Pass.shader->GetShaderID(), "BoneMatrices");
				glUniformMatrix4fv(boneMatLoc, 1, GL_FALSE, glm::value_ptr(id));
			}
			glDrawElements(GL_TRIANGLES, attrib->GetVertexCount(), GL_UNSIGNED_INT, 0);
		}
		
	}

}
