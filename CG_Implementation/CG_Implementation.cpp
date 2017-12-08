#include "CG_Implementation.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#include <thread>

float guiVertices[]{
	-1, 1, -1.0,
	-1, -1, -1.0,
	1, -1, -1.0,
	1, 1, -1.0
};

float guiTexCoords[]{
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0
};

unsigned int guiIndices[]{
	0, 1, 3,
	1, 2, 3
};


float waterPlaneVert[]{
	-1, 0, 1,
	-1, 0, -1,
	1, 0, -1,
	1, 0, 1,
};
unsigned int waterPlaneIndices[]{
	0, 1, 3,
	1, 2, 3
};

using namespace GL_Engine;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

//Callbacks for key events
static uint8_t activeEnt = 0;
long long time_millis_camera = 0;
void CameraKeyEvent(GLuint Key, void* Parameter) {
	Camera *camera = static_cast<Camera*>(Parameter);
	auto speed = 20.0f;// metres per second
	auto time_diff_sec = time_millis_camera / (float) 1e6;
	auto amount = speed * time_diff_sec;
	float dX = Key == GLFW_KEY_A ? -amount : Key == GLFW_KEY_D ? amount : 0;
	float dY = Key == GLFW_KEY_LEFT_SHIFT ? amount : Key == GLFW_KEY_LEFT_CONTROL ? -amount : 0;
	float dZ = Key == GLFW_KEY_W ? -amount : Key == GLFW_KEY_S ? amount : 0;
	camera->TranslateCamera(glm::vec4(dX, dY, dZ, 1.0));

	amount *= 4;
	float yaw = Key == GLFW_KEY_Q ? amount : Key == GLFW_KEY_E ? -amount : 0;
	camera->YawBy(yaw);

	//if (Key == GLFW_KEY_Q)
	//	camera->ReflectCamera();

	float pitch = Key == GLFW_KEY_Z ? amount : Key == GLFW_KEY_X ? -amount : 0;
	camera->PitchBy(pitch);
}

void CubeKeyEvent(GLuint Key, void* Parameter) {
	LightUBO_Data *data = (LightUBO_Data*)Parameter;

	switch (Key) {
	
	case GLFW_KEY_LEFT :
		data->LightPosition[0] += 0.1f;
		break;
	case GLFW_KEY_RIGHT:
		data->LightPosition[0] -= 0.1f;
		break;
	case GLFW_KEY_UP:
		data->LightPosition[1] += 0.1f;
		break;
	case GLFW_KEY_DOWN:
		data->LightPosition[1] -= 0.1f;
		break;
	case GLFW_KEY_INSERT:
		data->LightPosition[2] += 0.1f;
		break;
	case GLFW_KEY_DELETE:
		data->LightPosition[2] -= 0.1f;
		break;
	}
}

static bool wireframe = false;
void WireframeEvent(GLuint Key, void *Parameter) {
	wireframe = !wireframe;
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
}

CG_Implementation::CG_Implementation(){
}


int CG_Implementation::run(){
	
	initialise();


	while (!glfwWindowShouldClose(windowProperties.window)){
		uint64_t time_diff = FramerateStopwatch.MeasureTime().count();
		double second_diff = time_diff / 1.0e6;
		double fps = 1.0 / second_diff;
		char title[50];
		sprintf_s(title, "FPS: %f", fps);
		glfwSetWindowTitle(windowProperties.window, title);
		
		time_millis_camera = CameraStopwatch.MeasureTime().count();
		keyHandler.Update(windowProperties.window);
		barrel.GetTransformMatrix();
		kitchen.GetTransformMatrix();
		nanosuit.GetTransformMatrix();
		sun.GetTransformMatrix();
		dragon.GetTransformMatrix();
		DragonRiggedModel->GetTransformMatrix();
		DragonRiggedModel->Update();
		particleSystem->GetTransformMatrix();
		particleSystem->UpdateTime(second_diff);
		time += (float)second_diff;

		
		//Clear screen
		
		glEnable(GL_CLIP_DISTANCE0);
		camera_ubo_data.ClippingPlane[3] = 0;
		camera_ubo_data.ClippingPlane[1] = 1;
		camera.ReflectCamera();
		UpdateCameraUBO();
		water.Deactivate();
		WaterFBO->Bind(0);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		renderer->Render();

		camera_ubo_data.ClippingPlane[1] = -1;
		camera.ReflectCamera();
		UpdateCameraUBO();
		water.Deactivate();
		WaterFBO->Bind(1);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		renderer->Render();
		WaterFBO->Unbind();
		const GLenum CAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		ppFBO->Bind(2, CAttachments);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glDisable(GL_CLIP_DISTANCE0);
		camera_ubo_data.ClippingPlane[3] = 1000;
		water.Activate();
		renderer->Render();
		ppFBO->Unbind();

		postprocessPipeline.Process();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		guiRenderer->Render();

	
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
	sun.SetPosition(glm::vec3(light_ubo_data.LightPosition[0], light_ubo_data.LightPosition[1], light_ubo_data.LightPosition[2]));

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

	camera_ubo_data.ClippingPlane[0] = 0;
	camera_ubo_data.ClippingPlane[1] = 1;
	camera_ubo_data.ClippingPlane[2] = 0;
	camera_ubo_data.ClippingPlane[3] = 0;

	CG_Data::UBO *com_ubo = new CG_Data::UBO((void*)&camera_ubo_data, sizeof(camera_ubo_data));
	CG_Data::UBO *light_ubo = new CG_Data::UBO((void*)&light_ubo_data, sizeof(light_ubo_data));
	memcpy(light_ubo_data.LightColour, glm::value_ptr(glm::vec3(1, 1, 1)), sizeof(float) * 3);

	renderer = std::make_unique<Renderer>();
	DragonRenderer = std::make_unique<Renderer>();
	guiRenderer = std::make_unique<Renderer>();
	renderer->AddUBO(com_ubo);
	renderer->AddUBO(light_ubo);
	//Set up shader using a Shader object
	basicShader.RegisterShaderStageFromFile(vertexLoc, GL_VERTEX_SHADER);
	basicShader.RegisterShaderStageFromFile(fragLoc, GL_FRAGMENT_SHADER);
	basicShader.RegisterAttribute("vPosition", 0);
	basicShader.RegisterAttribute("vNormal", 2);
	basicShader.RegisterAttribute("TexCoord", 1);
	basicShader.RegisterAttribute("vTangeant", 3);
	basicShader.RegisterAttribute("vBitangeant", 4);
	basicShader.RegisterTextureUnit("diffuseTexture", 0);
	basicShader.RegisterTextureUnit("normalTexture", 1);
	basicShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	basicShader.RegisterUBO(std::string("LightData"), light_ubo);
	auto translate_uboBasic = basicShader.RegisterUniform("model");
	basicShader.CompileShader();

	waterShader.RegisterShaderStageFromFile(waterVLoc.c_str(), GL_VERTEX_SHADER);
	waterShader.RegisterShaderStageFromFile(waterFLoc.c_str(), GL_FRAGMENT_SHADER);
	waterShader.RegisterAttribute("vPosition", 0);
	waterShader.RegisterTextureUnit("reflectionTexture", 0);
	waterShader.RegisterTextureUnit("refractionTexture", 1);
	waterShader.RegisterTextureUnit("dudvMap", 2); 
	waterShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
//	waterShader.RegisterUBO(std::string("LightData"), light_ubo);
	auto waterTimeUniform = waterShader.RegisterUniform("Time");
	waterShader.CompileShader();

	nanosuitShader.RegisterShaderStageFromFile(nanosuitVShader.c_str(), GL_VERTEX_SHADER);
	nanosuitShader.RegisterShaderStageFromFile(nanosuitFShader.c_str(), GL_FRAGMENT_SHADER);
	nanosuitShader.RegisterAttribute("vPosition", 0);
	nanosuitShader.RegisterAttribute("vNormal", 2);
	nanosuitShader.RegisterAttribute("TexCoord", 1);
	nanosuitShader.RegisterAttribute("vTangeant", 3);
	nanosuitShader.RegisterAttribute("vBitangeant", 4);
	nanosuitShader.RegisterTextureUnit("diffuseTexture", 0);
	nanosuitShader.RegisterTextureUnit("normalTexture", 1);
	nanosuitShader.RegisterTextureUnit("specularTexture", 2);
	nanosuitShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	nanosuitShader.RegisterUBO(std::string("LightData"), light_ubo);
	translate_ubo = nanosuitShader.RegisterUniform("model");
	nanosuitShader.CompileShader();

	RiggedDragonShader.RegisterShaderStageFromFile(RiggedDragonVShader.c_str(), GL_VERTEX_SHADER);
	RiggedDragonShader.RegisterShaderStageFromFile(RiggedDragonFShader.c_str(), GL_FRAGMENT_SHADER);
	RiggedDragonShader.RegisterAttribute("vPosition", 0);
	RiggedDragonShader.RegisterAttribute("vNormal", 2);
	RiggedDragonShader.RegisterAttribute("TexCoord", 1);
	RiggedDragonShader.RegisterAttribute("vTangeant", 3);
	RiggedDragonShader.RegisterAttribute("vBitangeant", 4);
	RiggedDragonShader.RegisterAttribute("BoneIDs", 5);
	RiggedDragonShader.RegisterAttribute("BoneWeights", 6);
	RiggedDragonShader.RegisterTextureUnit("diffuseTexture", 0);
	RiggedDragonShader.RegisterTextureUnit("normalTexture", 1);
	RiggedDragonShader.RegisterTextureUnit("specularTexture", 2);
	RiggedDragonShader.RegisterUBO(std::string("CameraProjectionData"), com_ubo);
	RiggedDragonShader.RegisterUBO(std::string("LightData"), light_ubo);
	RiggedDragonShader.RegisterUniform("model");
	RiggedDragonShader.RegisterUniform("BoneMatrices");
	RiggedDragonShader.CompileShader();
	

	kitchenShader.RegisterShaderStageFromFile(kitchenVLoc.c_str(), GL_VERTEX_SHADER);
	kitchenShader.RegisterShaderStageFromFile(kitchenFLoc.c_str(), GL_FRAGMENT_SHADER);
	kitchenShader.RegisterAttribute("vPosition", 0);
	kitchenShader.RegisterAttribute("TexCoord", 1);
	kitchenShader.RegisterAttribute("vNormal", 2);
	kitchenShader.RegisterTextureUnit("diffuseTexture", 0);
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

	guiShader.RegisterShaderStageFromFile(guiVLoc.c_str(), GL_VERTEX_SHADER);
	guiShader.RegisterShaderStageFromFile(guiFLoc.c_str(), GL_FRAGMENT_SHADER);
	guiShader.RegisterAttribute("vPosition", 0);
	guiShader.RegisterAttribute("TexCoord", 1);
	guiShader.RegisterTextureUnit("image", 0);
	guiShader.RegisterTextureUnit("brightness", 1);
	guiShader.CompileShader();

	LoadModels();
	
	//Initialise camera
	camera.SetCameraPosition(glm::vec4(0, 10, -3, 1.0));
	camera.SetProjectionMatrix(0.01f, 1000.0f, 70.0f, (float)windowProperties.width / (float)windowProperties.height);

	

	//Set the update callbacks for the various uniforms using Lambda functions
	auto FloatLambda = [](const CG_Data::Uniform &u) {glUniform1fv(u.GetID(), 1, static_cast<const GLfloat*>(u.GetData())); };
	auto MatrixLambda = [](const CG_Data::Uniform &u) {glUniformMatrix4fv(u.GetID(), 1, GL_FALSE, static_cast<const GLfloat*>(u.GetData())); };
	translate_ubo->SetUpdateCallback(MatrixLambda);
	translate_uboBasic->SetUpdateCallback(MatrixLambda);
	waterTimeUniform->SetUpdateCallback(FloatLambda);

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

	//nodeModelIndex = dragon.AddData((void*)glm::value_ptr(dragon.TransformMatrix));
	//dragon.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : dragonAttributes) {
		//GLsizei dCount = (GLsizei)a->GetVertexCount();
		//RenderPass *dragonPass = renderer->AddRenderPass(&basicShader);
		//dragonPass->SetDrawFunction([dCount]() {glDrawElements(GL_TRIANGLES, dCount, GL_UNSIGNED_INT, 0); });
		//dragonPass->BatchVao = a;
		//std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(dragonPass->Textures));
	//	dragonPass->AddBatchUnit(&dragon);
		//dragonPass->AddDataLink(translate_ubo, nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}
	renderer->AddRenderPass(std::move(DragonRiggedModel->GenerateRenderpass(&RiggedDragonShader)));
	DragonRiggedModel->SetPosition(glm::vec3(0, 10, 0));


	nodeModelIndex = sun.AddData((void*)glm::value_ptr(sun.TransformMatrix));
	sun.SetPosition(glm::vec3(0, 0, 0));
	for (auto &a : sunAttributes) {
		GLsizei nCount = (GLsizei)a->GetVertexCount();
		RenderPass *sunPass = renderer->AddRenderPass(&kitchenShader);
		sunPass->SetDrawFunction([nCount]() {glDrawElements(GL_TRIANGLES, nCount, GL_UNSIGNED_INT, 0); });
		sunPass->BatchVao = a;
		std::move(a->ModelTextures.begin(), a->ModelTextures.end(), std::back_inserter(sunPass->Textures));
		sunPass->AddBatchUnit(&sun);
		sunPass->AddDataLink(translate_ubo, nodeModelIndex);	//Link the translate uniform to the transformation matrix of the entities
	}


	
	WaterFBO = std::make_unique<CG_Data::FBO>();
	auto reflectColAttach = WaterFBO->AddAttachment(CG_Data::FBO::AttachmentType::TextureAttachment, windowProperties.width, windowProperties.height);
	auto refractColAttach = WaterFBO->AddAttachment(CG_Data::FBO::AttachmentType::TextureAttachment, windowProperties.width, windowProperties.height);
	WaterFBO->AddAttachment(CG_Data::FBO::AttachmentType::DepthAttachment, windowProperties.width, windowProperties.height);
	auto reflectionTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(reflectColAttach)->GetTexture();
	auto refractionTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(refractColAttach)->GetTexture();
	reflectionTexture->SetUnit(GL_TEXTURE0);
	refractionTexture->SetUnit(GL_TEXTURE1);

	waterDUDVTexture = ModelLoader::LoadTexture(waterDUDV_loc, GL_TEXTURE2);
	waterVAO = std::make_shared<CG_Data::VAO>();
	waterVAO->BindVAO();
	auto vertexVBO = std::make_unique<CG_Data::VBO>(&waterPlaneVert[0], 12 * sizeof(float), GL_STATIC_DRAW);
	auto indices = std::make_unique<CG_Data::VBO>(&waterPlaneIndices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	vertexVBO->BindVBO();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	waterVAO->AddVBO(std::move(vertexVBO));
	waterVAO->AddVBO(std::move(indices));
	auto waterTimeIndex = water.AddData(&time);
	auto waterRenderPass = renderer->AddRenderPass(&waterShader);
	waterRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); });
	waterRenderPass->Textures.push_back(reflectionTexture);
	waterRenderPass->Textures.push_back(refractionTexture);
	waterRenderPass->Textures.push_back(waterDUDVTexture);
	waterRenderPass->BatchVao = waterVAO;
	waterRenderPass->AddDataLink(waterTimeUniform, waterTimeIndex);
	waterRenderPass->AddBatchUnit(&water);

	particleSystem = std::make_unique<ParticleSystem>();
	auto pRenderer = particleSystem->GenerateParticleSystem(50000, com_ubo, glm::vec3(0.0f, 69.0f, 60.0f), glm::vec3(0.0, -5.0, 20.0));
	renderer->AddRenderPass(std::move(pRenderer));

	ppFBO = std::make_unique<CG_Data::FBO>();
	auto FragColAttach = ppFBO->AddAttachment(CG_Data::FBO::AttachmentType::TextureAttachment, windowProperties.width, windowProperties.height);
	auto BrightColAttach = ppFBO->AddAttachment(CG_Data::FBO::AttachmentType::TextureAttachment, windowProperties.width, windowProperties.height);
	ppFBO->AddAttachment(CG_Data::FBO::AttachmentType::DepthAttachment, windowProperties.width, windowProperties.height);
	auto FragTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(FragColAttach)->GetTexture();
	auto BrightTexture = std::static_pointer_cast<CG_Data::FBO::TexturebufferObject>(BrightColAttach)->GetTexture();
	FragTexture->SetUnit(GL_TEXTURE0);
	BrightTexture->SetUnit(GL_TEXTURE1);

	CG_Data::Uniform *uni = postprocessPipeline.AddAttachment(PostProcessing::GaussianBlur);
	auto Tex = postprocessPipeline.Compile(BrightTexture, windowProperties.width, windowProperties.height);
	Tex->SetUnit(GL_TEXTURE1);

	guiVAO = std::make_shared<CG_Data::VAO>();
	guiVAO->BindVAO();
	vertexVBO = std::make_unique<CG_Data::VBO>(&guiVertices[0], 12 * sizeof(float), GL_STATIC_DRAW);
	auto texcoordVBO = std::make_unique<CG_Data::VBO>(&guiTexCoords[0], 8 * sizeof(float), GL_STATIC_DRAW);
	indices = std::make_unique<CG_Data::VBO>(&guiIndices[0], 6 * sizeof(unsigned int), GL_STATIC_DRAW, GL_ELEMENT_ARRAY_BUFFER);
	vertexVBO->BindVBO();
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	texcoordVBO->BindVBO();
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	guiVAO->AddVBO(std::move(vertexVBO));
	guiVAO->AddVBO(std::move(texcoordVBO));
	guiVAO->AddVBO(std::move(indices));
	auto guiRenderPass = guiRenderer->AddRenderPass(&guiShader);
	guiRenderPass->SetDrawFunction([]() {glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); });
	guiRenderPass->Textures.push_back(FragTexture);
	guiRenderPass->Textures.push_back(Tex);
	guiRenderPass->BatchVao = guiVAO;
	guiRenderPass->AddBatchUnit(&gui);

	
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
		keyHandler.AddKeyEvent(GLFW_KEY_INSERT, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		keyHandler.AddKeyEvent(GLFW_KEY_DELETE, KeyHandler::ClickType::GLFW_HOLD, KeyHandler::EventType::KEY_FUNCTION, &CubeKeyEvent, (void*)&light_ubo_data);
		keyHandler.AddKeyEvent(GLFW_KEY_F1, KeyHandler::ClickType::GLFW_CLICK, KeyHandler::EventType::KEY_FUNCTION, &WireframeEvent, (void*)nullptr);
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

	CameraStopwatch.Initialise();
	
}

void CG_Implementation::LoadModels() {

	barrelAttributes = mLoader.LoadModel(barrel_base, barrel_model, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);

	barrelAttributes[0]->AddTexture(mLoader.LoadTexture(barrel_diff_name, GL_TEXTURE0));
	barrelAttributes[0]->AddTexture(mLoader.LoadTexture(barrel_normal_name, GL_TEXTURE1));




	kitchenAttributes = mLoader.LoadModel(kitchen_base, kitchen_model, aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenSmoothNormals);



		nanosuitAttributes = mLoader.LoadModel(nanosuit_base, nanosuit_model, aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_GenSmoothNormals);

		sunAttributes = mLoader.LoadModel(sun_base, sun_model, aiProcess_CalcTangentSpace |
																aiProcess_Triangulate |
																aiProcess_JoinIdenticalVertices |
																aiProcess_SortByPType |
																aiProcess_GenSmoothNormals);
		dragonAttributes = mLoader.LoadModel(dragon_base, dragon_model, aiProcess_CalcTangentSpace |
																aiProcess_Triangulate |
																aiProcess_JoinIdenticalVertices |
																aiProcess_SortByPType |
																aiProcess_GenSmoothNormals);
//		dragonHierarchy = mLoader.LoadHierarchyModel(dragon_base, dragon_model, aiProcess_CalcTangentSpace |
	//															aiProcess_Triangulate |
		//														aiProcess_JoinIdenticalVertices |
			//													aiProcess_SortByPType |
				//												aiProcess_GenSmoothNormals);
		DragonRiggedModel = mLoader.LoadRiggedModel(dragon_base, dragon_model, aiProcess_CalcTangentSpace |
																aiProcess_Triangulate |
																aiProcess_JoinIdenticalVertices |
																aiProcess_SortByPType |
																aiProcess_GenSmoothNormals);
		mLoader.CleanUp();

}

//Cleanup
CG_Implementation::~CG_Implementation(){
	glfwTerminate();
}


