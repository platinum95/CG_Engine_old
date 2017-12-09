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
		return glm::transpose(tmp);
	}


	class MeshBone {
	public:
		MeshBone(const aiBone* _Bone) {
			this->OffsetMatrix = AiToGLMMat4(_Bone->mOffsetMatrix);
			this->Name = _Bone->mName.data;
		}
		const glm::mat4 &GetFinalTransform(const glm::mat4 &_GlobalInverse, const glm::mat4& nodeGlobalTransform) {
			this->FinalTransformation = _GlobalInverse * nodeGlobalTransform * this->OffsetMatrix;
			return this->FinalTransformation;
		}
		glm::mat4 OffsetMatrix;
		glm::mat4 FinalTransformation;
		std::string Name;
	};

	class SceneBone {
	public:
		SceneBone(const aiBone *_Bone) {
			this->Name = _Bone->mName.data;
		}
		void AddMeshBone(std::shared_ptr<MeshBone> _mBone) {
			this->meshBones.push_back(_mBone);
		}
		void UpdateBone(const glm::mat4 &GlobalInverse, const glm::mat4 &_GlobalTransform) {
			this->GlobalTransformation = _GlobalTransform;
			for (auto mb : meshBones) {
				mb->GetFinalTransform(GlobalInverse, this->GlobalTransformation);
			}
		}
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
		NodeAnimation(const aiNodeAnim *animNode, double _Length) {
			this->Name = animNode->mNodeName.data;
			this->AnimationLength = _Length;
			for (auto i = 0; i < animNode->mNumPositionKeys; i++) {
				aiVector3D pos = animNode->mPositionKeys[i].mValue;
				double time = animNode->mPositionKeys[i].mTime;
				this->Positions.push_back(std::make_pair(glm::vec3(pos.x, pos.y, pos.z), time));
			}
			for (auto i = 0; i < animNode->mNumScalingKeys; i++) {
				aiVector3D scale = animNode->mScalingKeys[i].mValue;
				double time = animNode->mScalingKeys[i].mTime;
				this->Scalings.push_back(std::make_pair(glm::vec3(scale.x, scale.y, scale.z), time));
			}
			for (auto i = 0; i < animNode->mNumRotationKeys; i++) {
				auto rot = animNode->mRotationKeys[i];
				double time = animNode->mPositionKeys[i].mTime;
				glm::quat rotQuat;
				rotQuat.x = rot.mValue.x;
				rotQuat.y = rot.mValue.y;
				rotQuat.z = rot.mValue.z;
				rotQuat.w = rot.mValue.w;
				this->Rotations.push_back(std::make_pair(rotQuat, time));
			}
		}
	};

	class SceneNode {
		public:
			SceneNode(const aiNode* _node) {
				this->NodeTransform = AiToGLMMat4(_node->mTransformation);
				this->Name = _node->mName.data;
			}
			void AddChild(std::shared_ptr<SceneNode> _node) {
				this->ChildNodes.push_back(_node);
			}
			void Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse) {
				this->GlobalTransform = ParentTransform * this->NodeTransform;
				for (auto sb : SceneBones) {
					sb->UpdateBone(GlobalInverse, this->GlobalTransform);
				}
				for (auto cn : ChildNodes) {
					cn->Update(this->GlobalTransform, GlobalInverse);
				}
			}
			void Update(const glm::mat4 &ParentTransform, const glm::mat4 &GlobalInverse, unsigned int AnimationID, double Time) {
				auto LocalMatrix = this->NodeTransform;
				if (Animation) {
					Time = fmod(Time, Animation->AnimationLength);
					glm::mat4 ScaleMatrix, RotateMatrix, TranslateMatrix;

					ScaleMatrix = this->GetInterpolatedScale(Animation->Scalings, Time);
					TranslateMatrix = this->GetInterpolatedTranslate(Animation->Positions, Time);
					RotateMatrix = this->GetInterpolatedRotate(Animation->Rotations, Time);

					LocalMatrix = TranslateMatrix * RotateMatrix * ScaleMatrix;
				}
				this->GlobalTransform = ParentTransform * LocalMatrix;
				for (auto sb : SceneBones) {
					sb->UpdateBone(GlobalInverse, this->GlobalTransform);
				}
				for (auto cn : ChildNodes) {
					cn->Update(this->GlobalTransform, GlobalInverse, AnimationID, Time);
				}
			}
			glm::mat4 GetInterpolatedScale(std::vector<std::pair<glm::vec3, double>> Scalings, double time) {
				std::pair<glm::vec3, double> LowerScale, UpperScale;
				for (int i = 0; i < Scalings.size() - 1; i++) {
					if (time > Scalings[i].second && time < Scalings[i + 1].second) {
						LowerScale = Scalings[i];
						UpperScale = Scalings[i + 1];
						break;
					}
					if (i == Scalings.size() - 2) {
						return glm::scale(glm::mat4(1.0), Scalings[i + 1].first);
					}
				}

				double timeDiff = UpperScale.second - LowerScale.second;
				double timeNorm = time - LowerScale.second;
				double ratio = timeNorm / timeDiff;
				glm::vec3 interpolatedScale = (LowerScale.first * (float)(1.0 - ratio)) + (UpperScale.first * (float)ratio);
				return glm::scale(glm::mat4(1.0), interpolatedScale);

			}
			glm::mat4 GetInterpolatedTranslate(std::vector<std::pair<glm::vec3, double>> Translations, double time) {
				std::pair<glm::vec3, double> LowerScale, UpperScale;
				for (int i = 0; i < Translations.size() - 1; i++) {
					if (time > Translations[i].second && time < Translations[i + 1].second) {
						LowerScale = Translations[i];
						UpperScale = Translations[i + 1];
						break;
					}
					if (i == Translations.size() - 2) {
						return glm::translate(glm::mat4(1.0), Translations[i + 1].first);
					}
				}

				double timeDiff = UpperScale.second - LowerScale.second;
				double timeNorm = time - LowerScale.second;
				double ratio = timeNorm / timeDiff;
				glm::vec3 interpolatedTranslate = (LowerScale.first * (float)(1.0 - ratio)) + (UpperScale.first * (float)ratio);
				return glm::translate (glm::mat4(1.0), interpolatedTranslate);
			}
			glm::mat4 GetInterpolatedRotate(std::vector<std::pair<glm::quat, double>> Rotations, double time) {
				std::pair<glm::quat, double> LowerScale, UpperScale;
				for (int i = 0; i < Rotations.size() - 1; i++) {
					if (time > Rotations[i].second && time < Rotations[i + 1].second) {
						LowerScale = Rotations[i];
						UpperScale = Rotations[i + 1];
						break;
					}
					if (i == Rotations.size() - 2) {
						return glm::toMat4(Rotations[i + 1].first);
					}
				}
				double timeDiff = UpperScale.second - LowerScale.second;
				double timeNorm = time - LowerScale.second;
				double ratio = timeNorm / timeDiff;
				glm::quat interp = glm::slerp(LowerScale.first, UpperScale.first, (float)ratio);
				return glm::toMat4(interp);
			}

			std::vector<std::shared_ptr<SceneBone>> SceneBones;
			std::vector<std::shared_ptr<SceneNode>> ChildNodes;
			std::shared_ptr<NodeAnimation> Animation{ nullptr };
			glm::mat4 NodeTransform, GlobalTransform;
			std::string Name;
	};

	class Skeleton {
	public:
		Skeleton(std::shared_ptr<SceneNode> _Root, std::map<std::string, std::shared_ptr<SceneNode>> SkeletonNodeMap){

			this->rootNode = _Root;
			this->GlobalInverseMatrix = glm::inverse(_Root->NodeTransform);
			this->NodeMap = SkeletonNodeMap;
		}
		std::shared_ptr<SceneNode> rootNode;
		void Update() {
			rootNode->Update(glm::mat4(1.0f), GlobalInverseMatrix);
		}
		void Update(unsigned int AnimationID, double Time) {
			rootNode->Update(glm::mat4(1.0f), GlobalInverseMatrix, AnimationID, Time);
		}
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
		RiggedModel(std::unique_ptr<Skeleton> _Rig, ModelAttribList &&_AttributeList) {
			this->ModelRig = std::move(_Rig);

			this->ModelAttributes = std::forward<ModelAttribList>(_AttributeList);

			Orientation = glm::quat(1, 0, 0, 0);
			eData.push_back(glm::value_ptr(this->TransformMatrix));
			this->ModelRig->Update();
		}
		~RiggedModel() {};
		std::unique_ptr<RenderPass> GenerateRenderpass(Shader* _Shader) {
			std::unique_ptr<RenderPass> renderPass = std::make_unique<RenderPass>();
			renderPass->renderFunction = RiggedModelRenderer;
			renderPass->shader = _Shader;
			renderPass->Data = (void*)this;
			return std::move(renderPass);
		}
		void Update() {
			this->ModelRig->Update();
		}
		void Update(unsigned int AnimationID, double Time) {
			this->ModelRig->Update(AnimationID, Time);
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


}