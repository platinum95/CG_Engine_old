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

static GLfloat vertices[] = {
	-1.0f, -1.0f, 0.0f,		//A
	1.0f, -1.0f, 0.0f,		//B
	-1.0f, 1.0f, 0.0f,		//D

	1.0f, -1.0f, 0.0f,		//B
	1.0f, 1.0f, 0.0f,		//C
	-1.0f, 1.0f, 0.0f,		//D


	-1.0f, -1.0f, 2.0f,		//A
	1.0f, -1.0f, 2.0f,		//B
	-1.0f, 1.0f, 2.0f,		//D

	1.0f, -1.0f, 2.0f,		//B
	1.0f, 1.0f, 2.0f,		//C
	-1.0f, 1.0f, 2.0f,		//D

	
	1.0f, 1.0f, 2.0f,		//A
	1.0f, -1.0f, 2.0f,		//B
	1.0f, 1.0f, 0.0f,		//D

	1.0f, 1.0f, 0.0f,		//B
	1.0f, -1.0f, 2.0f,		//C
	1.0f, -1.0f, 0.0f,		//D


	-1.0f, 1.0f, 2.0f,		//A
	-1.0f, -1.0f, 2.0f,		//B
	-1.0f, 1.0f, 0.0f,		//D

	-1.0f, 1.0f, 0.0f,		//B
	-1.0f, -1.0f, 2.0f,		//C
	-1.0f, -1.0f, 0.0f  	//D

};
// Create a color array that identfies the colors of each vertex (format R, G, B, A)
static GLfloat colors[] = {
	1.0f, 0.0f, 0.0f, 1.0f,	//RED  
	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 1.0f, 0.0f, 1.0f, //YELLOW

	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 0.0f, 0.0f, 1.0f,	//YELLOW
	1.0f, 1.0f, 0.0f, 1.0f,	//YELLOW

	1.0f, 0.0f, 0.0f, 1.0f,	//RED  
	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 1.0f, 0.0f, 1.0f, //YELLOW

	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 0.0f, 0.0f, 1.0f,	//YELLOW
	1.0f, 1.0f, 0.0f, 1.0f,	//YELLOW

	1.0f, 0.0f, 0.0f, 1.0f,	//RED  
	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 1.0f, 0.0f, 1.0f, //YELLOW

	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 0.0f, 0.0f, 1.0f,	//YELLOW
	1.0f, 1.0f, 0.0f, 1.0f,	//YELLOW

	1.0f, 0.0f, 0.0f, 1.0f,	//RED  
	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 1.0f, 0.0f, 1.0f, //YELLOW

	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 0.0f, 0.0f, 1.0f,	//YELLOW
	1.0f, 1.0f, 0.0f, 1.0f	//YELLOW


};

static GLfloat translate[] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 0
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
	Camera camera[4];
	KeyHandler keyHandler;
	float time{0};

	Entity entityList[4];
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Hierarchy> hierarchy;
}; 

