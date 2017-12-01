#version 330
in vec2 PassTexCoord;

out vec4 FragColour;

uniform sampler2D image, brightness;

void main(){
	vec2 tex = vec2(PassTexCoord.x, 1-PassTexCoord.y);
	FragColour = texture(brightness, tex);
}