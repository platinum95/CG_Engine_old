#version 330

in vec3 vPosition;
in vec3 fNormal;
uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 PassColour;
#define SCALE_FACTOR 0.5
mat4 scale = mat4(  SCALE_FACTOR, 0, 0, 0,
					0, SCALE_FACTOR, 0 , 0,
					0, 0, SCALE_FACTOR, 0,
					0, 0, 0, 1   );

void main(){
	float c = (1.0 + cos(time)) / 2.0;
	PassColour = vec4(fNormal.r, fNormal.g * c, fNormal.b * c, 1.0);
    gl_Position =  projection * view * model * vec4(vPosition, 1.0);
}