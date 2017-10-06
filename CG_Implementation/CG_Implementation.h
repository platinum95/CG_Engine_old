#pragma once

#include "CG_Engine.h"
#include "Shader.h"
#include "CG_Data.h"

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
	-1.0f, 1.0f, 0.0f		//D
};
// Create a color array that identfies the colors of each vertex (format R, G, B, A)
static GLfloat colors[] = {
	1.0f, 0.0f, 0.0f, 1.0f,	//RED  
	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 1.0f, 0.0f, 1.0f, //YELLOW

	1.0f, 1.0f, 0.0f, 1.0f,	//RED
	1.0f, 0.0f, 0.0f, 1.0f,	//YELLOW
	1.0f, 1.0f, 0.0f, 1.0f,	//YELLOW
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
	CG_Data::VBO *testVBO;
	CG_Data::VAO *testVAO;
}; 

