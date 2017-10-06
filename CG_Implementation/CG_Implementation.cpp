#include "CG_Implementation.h"


using namespace GL_Engine;
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CG_Implementation::CG_Implementation(){
}

int CG_Implementation::run(){
	
	initialise();
	
	while (!glfwWindowShouldClose(windowProperties.window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(basicShader.GetShaderID());
		glBindVertexArray(testVAO->GetID());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(windowProperties.window);
	//	glfwPollEvents();
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
	
	basicShader.AddShaderStageFromFile(vertexLoc, GL_VERTEX_SHADER);
	basicShader.AddShaderStageFromFile(fragLoc, GL_FRAGMENT_SHADER);
	basicShader.CompileShader();


	testVAO = new CG_Data::VAO();
	testVAO->BindVAO();

	testVBO = new CG_Data::VBO();
	testVBO->BindVBO();
	const GLuint vboID = testVBO->GetID();
	testVBO->SetVBOData(vertices, sizeof(vertices), GL_STATIC_DRAW);
	//float *ar = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	//GLuint positionID = glGetAttribLocation(basicShader.GetShaderID(), "vPosition");
	//GLuint colorID = glGetAttribLocation(basicShader.GetShaderID(), "fColor");
	//// Have to enable this
	//glEnableVertexAttribArray(positionID);
	//// Tell it where to find the position data in the currently active buffer (at index positionID)
	//glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//// Similarly, for the color data.
	//glEnableVertexAttribArray(colorID);
	//glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(6 * 3 * sizeof(GLfloat)));


	//	glBindAttribLocation(basicShader.GetShaderID(), 0, "vPosition");
	//	glBindAttribLocation(basicShader.GetShaderID(), 1, "fColor");
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(6 * 3 * sizeof(GL_FLOAT)));


}


CG_Implementation::~CG_Implementation(){
	glfwTerminate();
}
