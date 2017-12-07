#pragma once

#include "CG_Engine.h"
#include "Shader.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Renderer.h"
#include "Cubemap.h"
#include "ParticleSystem.h"
#include "Time.h"
#include "PostProcessing.h"
#include "ModelLoader.h"

static GL_Engine::Properties::GLFWproperties windowProperties = {
	1280,			//Width
	720,			//Height
	"Test Window",	//Title
	nullptr,
	nullptr,
	nullptr,
	false			//Fullscreen
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
	Shader basicShader, SkyboxShader, kitchenShader, nanosuitShader, guiShader, waterShader, RiggedDragonShader;
	CG_Engine engine;
	std::shared_ptr<CG_Data::VAO> VAO, guiVAO, waterVAO;
	CG_Data::Uniform *translate_ubo;
	Camera camera;
	KeyHandler keyHandler;
	float time{0};
	ModelLoader mLoader;
	std::unique_ptr<CG_Data::FBO> WaterFBO, ppFBO;

//	Hierarchy::HNode nodes[5];

//	Entity entityList[5];
	std::unique_ptr<Renderer> renderer, guiRenderer, DragonRenderer;
//	std::unique_ptr<Hierarchy> hierarchy;
	std::pair<std::unique_ptr<Hierarchy>, std::vector<AttribNodePair>> dragonHierarchy;
	std::unique_ptr<RiggedModel> DragonRiggedModel;
	Stopwatch<std::chrono::microseconds> CameraStopwatch, FramerateStopwatch;
	std::unique_ptr<Hierarchy> DragonHierarchy;
	ModelAttribList barrelAttributes, kitchenAttributes, nanosuitAttributes, sunAttributes, dragonAttributes;
	Entity barrel, kitchen, nanosuit, gui, water, sun, dragon;
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
	std::string sun_base = ModelBase + "sun/";
	std::string sun_model = "sun.obj";
	std::string waterDUDV_loc = ModelBase + "water/waterDUDV.png";
	std::string dragon_base = "assets/models/dragon/";
	std::string dragon_model = "dragon_blender.dae";

	CameraUBO_Data camera_ubo_data;
	LightUBO_Data light_ubo_data;

	std::string skyboxVLoc = "skyboxV.glsl", skyboxFLoc = "skyboxF.glsl";
	std::string waterVLoc = "waterV.glsl", waterFLoc = "waterF.glsl";
	std::string guiVLoc = "guiV.glsl", guiFLoc = "guiF.glsl";
	std::string kitchenVLoc = "kitchenV.glsl", kitchenFLoc = "kitchenF.glsl";
	std::string nanosuitVShader = "nanosuitV.glsl", nanosuitFShader = "nanosuitF.glsl";
	std::string RiggedDragonVShader = "RiggedDragonV.glsl", RiggedDragonFShader = "RiggedDragonF.glsl";
	std::vector<std::string> SkyboxTexLoc{ "./assets/skybox/right.png", "./assets/skybox/left.png", "./assets/skybox/top.jpg",
		"./assets/skybox/bottom.png", "./assets/skybox/back.png", "./assets/skybox/front.png" };
	std::unique_ptr<Cubemap> Skybox;

	std::unique_ptr<ParticleSystem> particleSystem;

	std::shared_ptr<CG_Data::Texture> waterDUDVTexture;

	PostProcessing postprocessPipeline;
};
	

