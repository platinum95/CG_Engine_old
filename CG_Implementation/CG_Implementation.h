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
	Shader basicShader;
	CG_Engine engine;
	CG_Data::VBO *vertexVBO, *colourVBO;
	std::shared_ptr<CG_Data::VAO> VAO;
	CG_Data::Uniform *time_ubo, *translate_ubo, *view_ubo, *projection_ubo;
	Camera camera;
	KeyHandler keyHandler;
	float time{0};

	Hierarchy::HNode nodes[5];

//	Entity entityList[5];
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Hierarchy> hierarchy;
}; 

