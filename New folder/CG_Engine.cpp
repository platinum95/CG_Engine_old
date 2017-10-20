#include "CG_Engine.h"
#include "Common.h"

namespace GL_Engine{

	CG_Engine::CG_Engine(){

	}


	CG_Engine::~CG_Engine(){
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		glViewport(0, 0, width, height);
	}
	

	bool CG_Engine::CG_CreateWindow(Properties::GLFWproperties *_DisplayProperties){

		if (!glfwInit()){
			return false;
		}
		if (_DisplayProperties->monitor == NULL)
			_DisplayProperties->monitor = _DisplayProperties->fullscreen ? glfwGetPrimaryMonitor() : NULL;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		_DisplayProperties->window = glfwCreateWindow(_DisplayProperties->width, _DisplayProperties->height,
													 _DisplayProperties->title, _DisplayProperties->monitor, 
													 _DisplayProperties->share); // Windowed
		if (!_DisplayProperties->window)
			return false;

		glfwMakeContextCurrent(_DisplayProperties->window);
		//Constants::DisplayProps = props;
		//Constants::GLFWProps = glfwProperties;
		return true;
	}

	bool CG_Engine::CG_StartGlad(Properties::GLADproperties * _GladProperties){
		_GladProperties->success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		return _GladProperties->success;
	}

	


}