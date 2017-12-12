#version 330 core

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in mat3 models;
in vec3 norms;
in vec2 PassTexCoord;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

uniform sampler2D GrassTexture; 

void main(){
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * LightColour; 

	//Diffuse
	vec3 norm = normalize(norms);   
	norm = normalize(models * norm); 

	vec3 lightDir = vec3(normalize(LightPosition_Viewspace.xyz - Pos_ViewSpace));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColour;
	diffuse = diffuse * Brightness;

	//Specular
	vec3 CamDir = normalize(vec3(0,0,0) - Pos_ViewSpace);
	vec3 ReflectDir = reflect(-lightDir, norm);
	float SpecAmount = pow(max(dot(CamDir, ReflectDir), 0.0), 16);
	vec3 SpecularComponent = 0.99 * SpecAmount * LightColour;  



	//Output
	vec3 result = (ambient + diffuse + SpecularComponent) * texture(GrassTexture, PassTexCoord).xyz;//(0.6, 0.1, 0.4);
	FragColour = vec4(result, 1.0);
	BrightColour = vec4(0.0);
}