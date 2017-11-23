#pragma once

#include "CG_Engine.h"
#include "Shader.h"
#include "CG_Data.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Entity.h"
#include "Renderer.h"
#include "Cubemap.h"

static GL_Engine::Properties::GLFWproperties windowProperties = {
	1280,			//Width
	720,			//Height
	"Test Window",	//Title
	nullptr,
	nullptr,
	nullptr,
	false			//Fullscreen
};


struct CameraUBO_Data {
	float ViewMatrix[16];
	float ProjectionMatrix[16];
	float PV_Matrix[16];
	float CameraPosition[4];
	float CameraOrientation[4];
};

struct LightUBO_Data {
	float LightPosition[4];
	float LightColour[3];
	float LightBrightness;
};

static const char* vertexLoc = "v.glsl";
static const char* fragLoc = "f.glsl";

using namespace GL_Engine;
class CG_Implementation
{
public:
	CG_Implementation();
	~CG_Implementation();

	int run();

private:
	void initialise();
	void LoadModels();
	void UpdateCameraUBO();
	Shader basicShader, SkyboxShader, kitchenShader, nanosuitShader;
	CG_Engine engine;
	std::shared_ptr<CG_Data::VAO> VAO;
	CG_Data::Uniform *translate_ubo;
	Camera camera;
	KeyHandler keyHandler;
	float time{0};
	CG_Data::ModelLoader mLoader;

//	Hierarchy::HNode nodes[5];

//	Entity entityList[5];
	std::unique_ptr<Renderer> renderer;
//	std::unique_ptr<Hierarchy> hierarchy;


	CG_Data::ModelAttribList barrelAttributes, kitchenAttributes, nanosuitAttributes;
	Entity barrel, kitchen, nanosuit;
	std::string AssetBase = "./assets/";
	std::string ModelBase = AssetBase + "models/";

	std::string barrel_base = ModelBase + "barrel/";
	std::string barrel_model = "barrel.obj";
	std::string barrel_diff_name = barrel_base + "textures/barrel.png";
	std::string barrel_normal_name = barrel_base + "textures/barrelNormal.png";
	std::string kitchen_base = "assets/models/kitchen/";
	std::string kitchen_model = "kitchen.obj";
	std::string nanosuit_base = ModelBase + "nanosuit/";
	std::string nanosuit_model = "nanosuit.obj";

	CameraUBO_Data camera_ubo_data;
	LightUBO_Data light_ubo_data;

	std::string skyboxVLoc = "skyboxV.glsl", skyboxFLoc = "skyboxF.glsl"; 
	std::string kitchenVLoc = "kitchenV.glsl", kitchenFLoc = "kitchenF.glsl";
	std::string nanosuitVShader = "nanosuitV.glsl", nanosuitFShader = "nanosuitF.glsl";
	std::vector<std::string> SkyboxTexLoc{ "./assets/skybox/right.png", "./assets/skybox/left.png", "./assets/skybox/top.jpg",
		"./assets/skybox/bottom.png", "./assets/skybox/back.png", "./assets/skybox/front.png" };
	std::unique_ptr<Cubemap> Skybox;
};
	

