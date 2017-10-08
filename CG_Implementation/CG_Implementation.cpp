#include "CG_Implementation.h"
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

using namespace GL_Engine;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CG_Implementation::CG_Implementation(){
}

int CG_Implementation::run(){
	
	initialise();
	glEnable(GL_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(windowProperties.window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(basicShader.GetShaderID());
		float time = (float)clock() / (float)CLOCKS_PER_SEC;
		glUniform1f(time_ubo->GetID(), (GLfloat) time);
		translate[14] = 0;
		glUniformMatrix4fv(translate_ubo->GetID(), 1, GL_FALSE, translate);
		camera.SetCameraPosition(glm::vec4(sin(time), 0, -1 + sin(time), 1));
		float* viewAr = (float*)glm::value_ptr(camera.GetViewMatrix());
		const float* projAr = (const float*)glm::value_ptr(camera.GetProjectionMatrix());
		//viewAr[14] = -1 + sin(time);
		glUniformMatrix4fv(view_ubo->GetID(), 1, GL_FALSE, viewAr);
		glUniformMatrix4fv(projection_ubo->GetID(), 1, GL_FALSE, projAr);
		glBindVertexArray(VAO->GetID());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(windowProperties.window);
		glfwPollEvents();

	}
	return 0;
}

void CG_Implementation::initialise(){
	
	if (!engine.CG_CreateWindow(&windowProperties)){
		throw std::runtime_error("Error initialising GLFW!");
	}
	
	Properties::GLADproperties gladProps;
	if (!engine.CG_StartGlad(&gladProps)){
		throw std::runtime_error("Error initialising GLAD!");
	}



	basicShader.RegisterShaderStageFromFile(vertexLoc, GL_VERTEX_SHADER);
	basicShader.RegisterShaderStageFromFile(fragLoc, GL_FRAGMENT_SHADER);
	basicShader.RegisterAttribute("vPosition", 0);
	basicShader.RegisterAttribute("fColor", 1);
	time_ubo = basicShader.RegisterUniform("time");
	translate_ubo = basicShader.RegisterUniform("model");
	view_ubo = basicShader.RegisterUniform("view");
	projection_ubo = basicShader.RegisterUniform("projection");
	basicShader.CompileShader();

	VAO = new CG_Data::VAO();
	VAO->BindVAO();

	vertexVBO = new CG_Data::VBO();
	vertexVBO->BindVBO();
	vertexVBO->SetVBOData(vertices, sizeof(vertices), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	colourVBO = new CG_Data::VBO();
	colourVBO->BindVBO();
	colourVBO->SetVBOData(colors, sizeof(colors), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	camera.SetCameraPosition(glm::vec4(0, 0, 0, 1.0));
	camera.SetProjectionMatrix(0.01, 100.0, 70, 800.0 / 600.0);


}


CG_Implementation::~CG_Implementation(){
	delete vertexVBO;
	delete colourVBO;
	delete VAO;
	basicShader.~Shader();
	glfwTerminate();
}
