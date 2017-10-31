#include "CG_Implementation.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "teapot.h"

#include <thread>

using namespace GL_Engine;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//Callbacks for key events
static uint8_t activeEnt = 0;
void CameraKeyEvent(GLuint Key, void* Parameter) {
	Camera *camera = static_cast<Camera*>(Parameter);
	auto amount = 0.5f;
	float dX = Key == GLFW_KEY_A ? -amount : Key == GLFW_KEY_D ? amount : 0;
	float dY = Key == GLFW_KEY_LEFT_SHIFT ? amount : Key == GLFW_KEY_LEFT_CONTROL ? -amount : 0;
	float dZ = Key == GLFW_KEY_W ? -amount : Key == GLFW_KEY_S ? amount : 0;
	camera->TranslateCamera(glm::vec4(dX, dY, dZ, 1.0));

	float yaw = Key == GLFW_KEY_Q ? amount : Key == GLFW_KEY_E ? -amount : 0;
	camera->YawBy(yaw);

	float pitch = Key == GLFW_KEY_Z ? 0.5f : Key == GLFW_KEY_X ? -0.5f : 0;
	camera->PitchBy(pitch);
}

void CubeKeyEvent(GLuint Key, void* Parameter) {
	Entity *entityList = (Entity*) Parameter;
	Entity *entity = &entityList[activeEnt];
	
	switch (Key) {
	case GLFW_KEY_1:
		activeEnt = 0;
		break;
	case GLFW_KEY_2:
		activeEnt = 1;
		break;
	case GLFW_KEY_3:
		activeEnt = 2;
		break;
	case GLFW_KEY_4:
		activeEnt = 3;
		break;
	case GLFW_KEY_LEFT :
		entity->Translate(glm::vec4(0.1f, 0, 0, 1.0));
		break;
	case GLFW_KEY_RIGHT:
		entity->Translate(glm::vec4(-0.1f, 0, 0, 1.0));
		break;
	case GLFW_KEY_UP:
		entity->Translate(glm::vec4(0, 0.1f, 0, 1.0));
		break;
	case GLFW_KEY_DOWN:
		entity->Translate(glm::vec4(0, -0.1f, 0, 1.0));
		break;
	case GLFW_KEY_INSERT:
		entity->Translate(glm::vec4(0, 0, 0.1f, 1.0));
		break;
	case GLFW_KEY_DELETE:
		entity->Translate(glm::vec4(0, 0, -0.1f, 1.0));
		break;

	case GLFW_KEY_O:
		entity->YawBy(0.5f);
		break;
	case GLFW_KEY_P:
		entity->YawBy(-0.5f);
		break;
	case GLFW_KEY_K:
		entity->PitchBy(0.5f);
		break;
	case GLFW_KEY_L:
		entity->PitchBy(-0.5f);
		break;
	case GLFW_KEY_M:
		entity->RollBy(0.5f);
		break;
	case GLFW_KEY_COMMA:
		entity->RollBy(-0.5f);
		break;

	case GLFW_KEY_KP_7:
		entity->ScaleBy(glm::vec3(0.99f));
		break;
	case GLFW_KEY_KP_9:
		entity->ScaleBy(glm::vec3(1.01f));
		break;
	case GLFW_KEY_KP_8:
		entity->ScaleBy(glm::vec3(0.9f, 1.0f, 1.0f));
		break;
	case GLFW_KEY_KP_2:
		entity->ScaleBy(glm::vec3(1.1f, 1.0f, 1.0f));
		break;

	}
}

CG_Implementation::CG_Implementation(){
}

int CG_Implementation::run(){
	
	initialise();

	while (!glfwWindowShouldClose(windowProperties.window)){
		keyHandler.Update(windowProperties.window);
		//Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		entityList[0].YawBy(0.3);

		renderer->Render();

		entityList[0].GetTransformMatrix();


		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//Swap buffers
		glfwSwapBuffers(windowProperties.window);
		glfwPollEvents();
		
	}
	return 0;
}

void CG_Implementation::initialise(){
	
	//Initialise window and Glad
	if (!engine.CG_CreateWindow(&windowProperties)){
		throw std::runtime_error("Error initialising GLFW!");
	}
	
	Properties::GLADproperties gladProps;
	if (!engine.CG_StartGlad(&gladProps)){
		throw std::runtime_error("Error initialising GLAD!");
	}

	renderer = std::make_unique<Renderer>();
	//Set up shader using a Shader object
	basicShader.RegisterShaderStageFromFile(vertexLoc, GL_VERTEX_SHADER);
	basicShader.RegisterShaderStageFromFile(fragLoc, GL_FRAGMENT_SHADER);
	basicShader.RegisterAttribute("vPosition", 0);
	basicShader.RegisterAttribute("fColor", 1);
	time_ubo = basicShader.RegisterUniform("time");
	translate_ubo = basicShader.RegisterUniform("model");
	view_ubo = basicShader.RegisterUniform("view");
	projection_ubo = basicShader.RegisterUniform("projection");
	basicShader.CompileShader();


	//Set up vertex data
	VAO = std::make_shared<CG_Data::VAO>();
	VAO->BindVAO();

	vertexVBO = new CG_Data::VBO();
	vertexVBO->BindVBO();
	vertexVBO->SetVBOData(teapot_vertex_points, sizeof(teapot_vertex_points), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	colourVBO = new CG_Data::VBO();
	colourVBO->BindVBO();
	colourVBO->SetVBOData(teapot_normals, sizeof(teapot_normals), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	
	//Initialise camera
	camera.SetCameraPosition(glm::vec4(0, 0, 0, 1.0));
	camera.SetProjectionMatrix(0.01f, 100.0f, 70.0f, 800.0f / 600.0f);

	{
	//Register key events
	keyHandler.AddKeyEvent(GLFW_KEY_W, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_A, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_S, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_D, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT_SHIFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT_CONTROL, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_Q, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_E, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_Z, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);
	keyHandler.AddKeyEvent(GLFW_KEY_X, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CameraKeyEvent, (void*)&camera);

	keyHandler.AddKeyEvent(GLFW_KEY_UP, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_DOWN, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_LEFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_RIGHT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_INSERT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_DELETE, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_O, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_P, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_K, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_L, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_M, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_COMMA, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_KP_7, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_KP_9, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_KP_8, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_KP_2, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_KP_5, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_1, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_2, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_3, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	keyHandler.AddKeyEvent(GLFW_KEY_4, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&entityList[0]);
	}
	//Initialise entities
	for(int i = 0; i < 4; i++){
		entityList[i].Translate(glm::vec3(0, 0, 0));
	}

	//Set the update callbacks for the various uniforms using Lambda functions

	auto MatrixLambda = [](const CG_Data::Uniform &u) {glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, static_cast<const GLfloat*>(u.GetData())); };
	translate_ubo->SetUpdateCallback(MatrixLambda);

	projection_ubo->SetUpdateCallback(MatrixLambda);

	view_ubo->SetUpdateCallback([](const CG_Data::Uniform &u)
	{glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, glm::value_ptr(((Camera*)u.GetData())->GetViewMatrix()));});


	const auto width = windowProperties.width, height = windowProperties.height;

	auto cam_link_index = entityList[0].AddData((void*) glm::value_ptr(camera.GetViewMatrix()));
	auto proj_link_index = entityList[0].AddData((void*)glm::value_ptr(camera.GetProjectionMatrix()));


	RenderPass *teapotPass = renderer->AddRenderPass(&basicShader);
	teapotPass->SetDrawFunction ([](){glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);});
	teapotPass->BatchVao = VAO;
	teapotPass->AddBatchUnit(&entityList[0]);
	teapotPass->AddDataLink(translate_ubo, 0);	//Link the translate uniform to the transformation matrix of the entities
	teapotPass->AddDataLink(view_ubo, cam_link_index);
	teapotPass->AddDataLink(projection_ubo, proj_link_index);
	glEnable(GL_DEPTH_TEST);


	hierarchy = std::make_unique<Hierarchy>();
	/*
	
	|_|_|	hand and fingers
	  |		arm
	  ^		root
	  
	*/
	auto root = hierarchy->InitialiseHierarchy({});
	root->Entity = &entityList[0];
	auto arm = hierarchy->AddChild(root, &entityList[1]);
}

//Cleanup
CG_Implementation::~CG_Implementation(){
	delete vertexVBO;
	delete colourVBO;
	basicShader.~Shader();
	glfwTerminate();
}


