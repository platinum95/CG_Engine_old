#include "CG_Implementation.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

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
	LightUBO_Data *data = (LightUBO_Data*)Parameter;

	switch (Key) {
	
	case GLFW_KEY_LEFT :
		data->LightPosition[1] += 0.1f;
		break;
	case GLFW_KEY_RIGHT:
		data->LightPosition[1] -= 0.1f;
		break;
	case GLFW_KEY_UP:
		data->LightPosition[0] += 0.1f;
		break;
	case GLFW_KEY_DOWN:
		data->LightPosition[0] -= 0.1f;
		break;


	}
}

CG_Implementation::CG_Implementation(){
}

int CG_Implementation::run(){
	
	initialise();

	while (!glfwWindowShouldClose(windowProperties.window)){
		keyHandler.Update(windowProperties.window);
		UpdateCameraUBO();

		//Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		static auto prev = std::chrono::high_resolution_clock::now();
		auto now = std::chrono::high_resolution_clock::now();

		auto diff = now - prev;
		auto tim = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
		double f = (double)tim;
		f /= 1000000000.0;
//		std::cout << 1.0/f << std::endl;
//		std::cout << nodes[4].GlobalMatrix[3][0] << std::endl;
		prev = std::chrono::high_resolution_clock::now();

		renderer->Render();

		barrel.GetTransformMatrix();
		kitchen.GetTransformMatrix();
		nanosuit.GetTransformMatrix();

		std::this_thread::sleep_for(std::chrono::milliseconds(3));
		//Swap buffers
		glfwSwapBuffers(windowProperties.window);
		glfwPollEvents();
		
	}
	return 0;
}

void CG_Implementation::UpdateCameraUBO() {

	memcpy(camera_ubo_data.ViewMatrix, glm::value_ptr(camera.GetViewMatrix()), sizeof(float) * 16);
	memcpy(camera_ubo_data.ProjectionMatrix, glm::value_ptr(camera.GetProjectionMatrix()), sizeof(float) * 16);
	glm::mat4 PV = camera.GetProjectionMatrix() * camera.GetViewMatrix();
	memcpy(camera_ubo_data.PV_Matrix, glm::value_ptr(PV), sizeof(float) * 16);
	memcpy(camera_ubo_data.CameraOrientation, glm::value_ptr(glm::vec4(camera.GetForwardVector(), 0.0)), sizeof(float) * 4);
	memcpy(camera_ubo_data.CameraPosition, glm::value_ptr(camera.GetCameraPosition()), sizeof(float) * 4);
	
//	memcpy(light_ubo_data.LightPosition, glm::value_ptr(camera.GetCameraPosition()), sizeof(float) * 4);
//	memcpy(light_ubo_data.LightPosition, glm::value_ptr(glm::vec4(0, 50, -50, 1.0)), sizeof(float) * 4);
	//memcpy(light_ubo_data.LightColour, glm::value_ptr(glm::vec3(1, 1, 1)), sizeof(float) * 3);
	light_ubo_data.LightBrightness = 1;
}

void CG_Implementation::initialise(){
	
	memcpy(light_ubo_data.LightPosition, glm::value_ptr(glm::vec4(0, 20, -20, 1.0)), sizeof(float) * 4);

	//Initialise window and Glad
	if (!engine.CG_CreateWindow(&windowProperties)){
		throw std::runtime_error("Error initialising GLFW!");
	}
	
	Properties::GLADproperties gladProps;
	if (!engine.CG_StartGlad(&gladProps)){
		throw std::runtime_error("Error initialising GLAD!");
	}


	CG_Data::UBO *com_ubo = new CG_Data::UBO((void*)&camera_ubo_data, sizeof(camera_ubo_data));
	CG_Data::UBO *light_ubo = new CG_Data::UBO((void*)&light_ubo_data, sizeof(light_ubo_data));
	memcpy(light_ubo_data.LightColour, glm::value_ptr(glm::vec3(1, 1, 1)), sizeof(float) * 3);

	renderer = std::make_unique<Renderer>();
	renderer->AddUBO(com_ubo);
	renderer->AddUBO(light_ubo);
	//Set up shader using a Shader object
	basicShader.RegisterShaderStageFromFile(vertexLoc, GL_VERTEX_SHADER);
	basicShader.RegisterShaderStageFromFile(fragLoc, GL_FRAGMENT_SHADER);
	basicShader.RegisterAttribute("vPosition", 0);
	basicShader.RegisterAttribute("vNormal", 1);
	basicShader.RegisterAttribute("TexCoord", 2);
	basicShader.RegisterAttribute("vTangeant", 3);
	basicShader.RegisterAttribute("vBitangeant", 4);
	basicShader.RegisterTextureUnit("diffuseTexture", 0);
	basicShader.RegisterTextureUnit("normalTexture", 1);
	basicShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	basicShader.RegisterUBO(std::string("LightData"), light_ubo);
	translate_ubo = basicShader.RegisterUniform("model");
	basicShader.CompileShader();

	nanosuitShader.RegisterShaderStageFromFile(nanosuitVShader.c_str(), GL_VERTEX_SHADER);
	nanosuitShader.RegisterShaderStageFromFile(nanosuitFShader.c_str(), GL_FRAGMENT_SHADER);
	nanosuitShader.RegisterAttribute("vPosition", 0);
	nanosuitShader.RegisterAttribute("vNormal", 1);
	nanosuitShader.RegisterAttribute("TexCoord", 2);
	nanosuitShader.RegisterAttribute("vTangeant", 3);
	nanosuitShader.RegisterAttribute("vBitangeant", 4);
	nanosuitShader.RegisterTextureUnit("diffuseTexture", 0);
	nanosuitShader.RegisterTextureUnit("normalTexture", 1);
	nanosuitShader.RegisterTextureUnit("specularTexture", 2);
	nanosuitShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	nanosuitShader.RegisterUBO(std::string("LightData"), light_ubo);
	translate_ubo = nanosuitShader.RegisterUniform("model");
	nanosuitShader.CompileShader();

	kitchenShader.RegisterShaderStageFromFile(kitchenVLoc.c_str(), GL_VERTEX_SHADER);
	kitchenShader.RegisterShaderStageFromFile(kitchenFLoc.c_str(), GL_FRAGMENT_SHADER);
	kitchenShader.RegisterAttribute("vPosition", 0);
	kitchenShader.RegisterAttribute("vNormal", 1);
	SkyboxShader.RegisterTextureUnit("diffuseTexture", 0);
	kitchenShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	kitchenShader.RegisterUBO(std::string("LightData"), light_ubo);
	translate_ubo = kitchenShader.RegisterUniform("model");
	kitchenShader.CompileShader();

	SkyboxShader.RegisterShaderStageFromFile(skyboxVLoc.c_str(), GL_VERTEX_SHADER);
	SkyboxShader.RegisterShaderStageFromFile(skyboxFLoc.c_str(), GL_FRAGMENT_SHADER);
	SkyboxShader.RegisterAttribute("vPosition", 1);
	SkyboxShader.RegisterTextureUnit("BoxTexture", 0);
	SkyboxShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	SkyboxShader.CompileShader();
	int loc = glGetAttribLocation(SkyboxShader.GetShaderID(), "vPosition");
	Skybox = std::make_unique<Cubemap>(SkyboxTexLoc, &SkyboxShader, renderer.get());

	LoadModels();
	
	//Initialise camera
	camera.SetCameraPosition(glm::vec4(0, 10, -3, 1.0));
	camera.SetProjectionMatrix(0.01f, 100.0f, 70.0f, (float)windowProperties.width / (float)windowProperties.height);

	

	//Set the update callbacks for the various uniforms using Lambda functions

	auto MatrixLambda = [](const CG_Data::Uniform &u) {glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, static_cast<const GLfloat*>(u.GetData())); };
	translate_ubo->SetUpdateCallback(MatrixLambda);

	const auto width = windowProperties.width, height = windowProperties.height;


	GLsizei bCount;
	auto nodeModelIndex = barrel.AddData((void*)glm::value_ptr(barrel.TransformMatrix));
	barrel.SetPosition(glm::vec3(0, 0, 0));
	bCount = (GLsizei)barrelAttributes[0]->GetVertexCount();
	RenderPass *barrelPass = renderer->AddRenderPass(&basicShader);
	barrelPass->SetDrawFunction([bCount]() {glDrawElements(GL_TRIANGLES, bCount, GL_UNSIGNED_INT, 0); });
	barrelPass->BatchVao = barrelAttributes[0];
	std::move(barrelAttributes[0]->ModelTextures.begin(), barrelAttributes[0]->ModelTextures.end(), std::back_inserter(barrelPass->Textures));
	barrelPass->AddBatchUnit(&barrel);
	barrelPass->AddDataLink(translate_ubo, nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	barrel.Translate(glm::vec3(3, 4,8));

	nodeModelIndex = kitchen.AddData((void*)glm::value_ptr(kitchen.TransformMatrix));
	kitchen.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : kitchenAttributes) {
		GLsizei kCount = (GLsizei)a->GetVertexCount();
		RenderPass *kitchenPass = renderer->AddRenderPass(&kitchenShader);
		kitchenPass->SetDrawFunction([kCount]() {glDrawElements(GL_TRIANGLES, kCount, GL_UNSIGNED_INT, 0); });
		kitchenPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(kitchenPass->Textures));
		kitchenPass->AddBatchUnit(&kitchen);
		kitchenPass->AddDataLink(translate_ubo, nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}
	kitchen.ScaleBy(glm::vec3(0.3, 0.3, 0.3));
	kitchen.Translate(glm::vec3(0, -2, 0));

	nodeModelIndex = nanosuit.AddData((void*)glm::value_ptr(nanosuit.TransformMatrix));
	nanosuit.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : nanosuitAttributes) {
		GLsizei nCount = (GLsizei)a->GetVertexCount();
		RenderPass *nanosuitPass = renderer->AddRenderPass(&nanosuitShader);
		nanosuitPass->SetDrawFunction([nCount]() {glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_INT, 0); });
		nanosuitPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(nanosuitPass->Textures));
		nanosuitPass->AddBatchUnit(&nanosuit);
		nanosuitPass->AddDataLink(translate_ubo, nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}
	//kitchen.ScaleBy(glm::vec3(0.3, 0.3, 0.3));
	//kitchen.Translate(glm::vec3(0, -2, 0));
	glEnable(GL_DEPTH_TEST);

	
	
	
	/*
	
	|_|_|	hand and fingers
	  |		arm
	  ^		root
	  
	*/
	/*
	hierarchy = std::make_unique<Hierarchy>();
	//Initialise joints
	auto *root_joint = new Hierarchy::HJoint(glm::vec3(0, 0, 0));
	auto *wrist_joint = new Hierarchy::HJoint(glm::vec3(0, 20, 0));
	auto *leftKnuckle_joint = new Hierarchy::HJoint(glm::vec3(20, 20, 0));
	auto *middleKnuckle_joint = new Hierarchy::HJoint(glm::vec3(0, 20, 0));
	auto *rightKnuckle_joint = new Hierarchy::HJoint(glm::vec3(-20, 20, 0));

	//Set up joint hierarchy
	root_joint->AddChild(wrist_joint);
	wrist_joint->AddChild(leftKnuckle_joint);
	wrist_joint->AddChild(middleKnuckle_joint);
	wrist_joint->AddChild(rightKnuckle_joint);

	//set root join
	hierarchy->SetRoot(root_joint);


	nodes[0].SetPosition(glm::vec3(0, 0, 0));
	nodes[1].SetPosition(glm::vec3(0, 0, 0));
	nodes[2].SetPosition(glm::vec3(0, 10, 0));
	nodes[3].SetPosition(glm::vec3(0, 10, 0));
	nodes[4].SetPosition(glm::vec3(0, 10, 0));

	root_joint->AddNode(&nodes[0]);
	wrist_joint->AddNode(&nodes[1]);
	leftKnuckle_joint->AddNode(&nodes[2]);
	middleKnuckle_joint->AddNode(&nodes[3]);
	rightKnuckle_joint->AddNode(&nodes[4]);

	hierarchy->InitialiseHierarchy();
	*/
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
		keyHandler.AddKeyEvent(GLFW_KEY_UP, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		keyHandler.AddKeyEvent(GLFW_KEY_DOWN, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		keyHandler.AddKeyEvent(GLFW_KEY_LEFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		keyHandler.AddKeyEvent(GLFW_KEY_RIGHT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		/*
		keyHandler.AddKeyEvent(GLFW_KEY_UP, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_DOWN, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_LEFT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_RIGHT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_INSERT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_DELETE, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_O, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_P, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_K, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_L, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_M, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_COMMA, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_Y, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_U, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_H, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_J, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_B, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_N, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_KP_7, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_KP_9, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_KP_8, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_KP_2, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_KP_5, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_1, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_2, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_3, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		keyHandler.AddKeyEvent(GLFW_KEY_4, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)hierarchy.get());
		*/
	}
	
}

void CG_Implementation::LoadModels() {

	barrelAttributes = mLoader.LoadModel(barrel_base, barrel_model, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	barrelAttributes[0]->AddTexture(mLoader.LoadTexture(barrel_diff_name, GL_TEXTURE0));
	barrelAttributes[0]->AddTexture(mLoader.LoadTexture(barrel_normal_name, GL_TEXTURE1));

	barrelAttributes[0]->BindVAO();
	auto mVBO = barrelAttributes[0]->GetVBO(barrelAttributes[0]->MeshIndex);
	mVBO->BindVBO();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	auto nVBO = barrelAttributes[0]->GetVBO(barrelAttributes[0]->NormalIndex);
	nVBO->BindVBO();
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	auto tVBO = barrelAttributes[0]->GetVBO(barrelAttributes[0]->TexCoordIndex);
	tVBO->BindVBO();
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	auto taVBO = barrelAttributes[0]->GetVBO(3);
	taVBO->BindVBO();
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(3);
	auto btaVBO = barrelAttributes[0]->GetVBO(4);
	btaVBO->BindVBO();
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(4);


	kitchenAttributes = mLoader.LoadModel(kitchen_base, kitchen_model, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

		for (auto a : kitchenAttributes) {
			a->BindVAO();
			mVBO = a->GetVBO(a->MeshIndex);
			mVBO->BindVBO();
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(0);
			nVBO = a->GetVBO(a->NormalIndex);
			nVBO->BindVBO();
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
			auto tVBO = a->GetVBO(a->TexCoordIndex);
			tVBO->BindVBO();
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(2);
			auto taVBO = a->GetVBO(3);
			taVBO->BindVBO();
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(3);
			auto btaVBO = a->GetVBO(4);
			btaVBO->BindVBO();
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(4);
		}

		nanosuitAttributes = mLoader.LoadModel(nanosuit_base, nanosuit_model, aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_GenSmoothNormals);
		for (auto a : nanosuitAttributes) {
			a->BindVAO();
			mVBO = a->GetVBO(a->MeshIndex);
			mVBO->BindVBO();
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(0);
			nVBO = a->GetVBO(a->NormalIndex);
			nVBO->BindVBO();
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(1);
			auto tVBO = a->GetVBO(a->TexCoordIndex);
			tVBO->BindVBO();
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(2);
			auto taVBO = a->GetVBO(3);
			taVBO->BindVBO();
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(3);
			auto btaVBO = a->GetVBO(4);
			btaVBO->BindVBO();
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(4);
		}
	
}

//Cleanup
CG_Implementation::~CG_Implementation(){
	glfwTerminate();
}


