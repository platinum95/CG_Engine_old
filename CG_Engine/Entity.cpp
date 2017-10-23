#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace GL_Engine {
	Entity::Entity() {
		Orientation = glm::quatLookAt(glm::vec3(0, 0, 1), Up);
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
//		for (const auto uniform : this->EntityUniforms) {
//			uniform.Update();
//		}
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

}
