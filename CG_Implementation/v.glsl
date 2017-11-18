#version 330


layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
};


in vec3 vPosition;
in vec3 fColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 PassColour;


void main(){
	PassColour = vec4(fColor, 1.0);
    gl_Position =  PV_Matrix * model * vec4(vPosition, 1.0);
}