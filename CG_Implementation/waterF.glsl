#version 330

in vec2 TexCoord;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

out vec4 frag_Colour;

void main(){
	frag_Colour = texture(reflectionTexture, TexCoord);
}

