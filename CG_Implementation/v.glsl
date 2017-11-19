#version 330


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
in vec3 fColor;
uniform mat4 model;
varying vec4 PassColour;
out vec3 Normal;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

void main(){
	Pos_ViewSpace = vec3(ViewMatrix * model * vec4(vPosition, 1.0));
	Normal = mat3(transpose(inverse( ViewMatrix * model))) * fColor;
	PassColour = vec4(fColor, 1.0);
	LightPosition_Viewspace = ViewMatrix * LightPosition;
    gl_Position =  PV_Matrix * model * vec4(vPosition, 1.0);
}