#version 330                                                                        
out vec4 FragColor;
in vec4 PassColour;

void main(){
	FragColor = (PassColour + 1.0) / 2.0;
}