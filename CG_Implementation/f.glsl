#version 330                                                                        
out vec4 FragColour;

in mat3 models;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in; 

void main(){
	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * LightColour; 

	vec3 normal2 = texture(normalTexture, fs_in.TexCoords).rgb;
	normal2 = normalize(normal2 * 2.0 - 1.0);   
	normal2 = normalize(fs_in.TBN * normal2); 

	vec3 norm = normal2;
	vec3 lightDir = vec3(normalize(LightPosition_Viewspace.xyz - Pos_ViewSpace));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColour;

	diffuse = diffuse * Brightness;
	vec3 result = (ambient + diffuse) * texture(diffuseTexture, fs_in.TexCoords).xyz;
	FragColour = vec4(result, 1.0);

}