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
out vec2 TexCoord;


void main(){
	vec4 vertexPos = vec4(vPosition * SCALE, 1);

	TexCoord = vec2((vPosition.x + 1.0 / 2.0), (vPosition.z + 1.0)/2.0);
	gl_Position = PV_Matrix * vertexPos + Brightness/Brightness;

}