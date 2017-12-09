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

	
	void sanityCheck(glm::mat4 &matrix) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (matrix[i][j] > 100 || matrix[i][j] < -100) {
					std::cout << "sanity failed, value " << matrix[i][j] << std::endl;
				}
			}
		}
	}

	void sanityCheck(glm::vec4 vect) {
		for (int i = 0; i < 4; i++) {
			if (vect[i] > 100 || vect[i] < -100) {
				std::cout << "sanity failed, value " << vect[i] << std::endl;
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
			std::vector<glm::mat4> boneMatrices((const size_t)56, glm::mat4(1.0));
			int i = 0;
			if (attrib->meshBones.size() > 0) {
				int i = 0;
				for (auto bone : attrib->meshBones) {
					boneMatrices.at(i++) = bone->FinalTransformation;
					sanityCheck(bone->FinalTransformation * glm::vec4(10, 10, 10, 1));
				}
				glUniformMatrix4fv(boneMatLoc, 56, GL_FALSE, glm::value_ptr(boneMatrices[0]));
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
