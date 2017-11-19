#version 330                                                                        
out vec4 FragColour;
varying vec4 PassColour;
in vec3 Normal;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};


void main(){
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * LightColour; 

	vec3 norm = normalize(Normal);
	vec3 lightDir = vec3(normalize(LightPosition_Viewspace.xyz - Pos_ViewSpace));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColour;

	diffuse = diffuse * Brightness;
	vec3 result = (ambient + diffuse) * vec3((PassColour+1.0)/2.0);
	FragColour = vec4(result, 1.0);

}