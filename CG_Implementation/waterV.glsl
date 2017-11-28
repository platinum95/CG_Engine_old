#version 330  

#define SCALE 500

layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
};

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

in vec3 vPosition;
out vec4 ClipspaceCoord;


void main(){
	vec4 vertexPos = vec4(vPosition * SCALE, 1);

	ClipspaceCoord = PV_Matrix * vertexPos;
	gl_Position = ClipspaceCoord;

}