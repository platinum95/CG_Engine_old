#version 330 

in vec3 textureCoords;
out vec4 fragColour;

uniform samplerCube BoxTexture;

void main(void){
    fragColour = texture(BoxTexture, textureCoords);
}