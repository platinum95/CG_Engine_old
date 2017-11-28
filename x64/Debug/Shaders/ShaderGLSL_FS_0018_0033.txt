#version 330
in vec2 PassTexCoord;

out vec4 FragColour;

uniform sampler2D image;

void main(){
	FragColour = texture(image, PassTexCoord);
}