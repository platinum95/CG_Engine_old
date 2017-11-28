R"(
#version 330 core

out vec4 frag_colour;

in float PassTime;
in vec3 PassColour;
in float PassOpacity;

void main(){
	frag_colour = vec4(PassColour, PassOpacity);
}

)"
