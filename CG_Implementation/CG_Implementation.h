#pragma once

#include "CG_Engine.h"
#include "Shader.h"
#include "CG_Data.h"
#include "Camera.h"
#include "InputHandler.h"
#include "Entity.h"
#include "Renderer.h"

static GL_Engine::Properties::GLFWproperties windowProperties = {
	800,			//Width
	600,			//Height
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
	Shader basicShader;
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

	CG_Data::ModelAttribList monkeyAttributes;
	Entity suzanne;
	std::string suzanne_loc = "assets/models/suzanne.dae";

	CameraUBO_Data camera_ubo_data;
	LightUBO_Data light_ubo_data;
}; 

