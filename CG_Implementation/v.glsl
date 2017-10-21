#version 330

in vec3 vPosition;
in vec3 fColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 PassColour;


void main(){
	PassColour = vec4(fColor, 1.0);
    gl_Position =  projection * view * model * vec4(vPosition, 1.0);
}