#version 330

#define WAVE_STRENGTH	0.01
#define WAVE_SCALE		6

in vec4 ClipspaceCoord;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform float Time;

in vec2 TexCoord;
out vec4 frag_Colour;

void main(){
	vec2 ndc = ClipspaceCoord.xy/ClipspaceCoord.w;
	ndc = vec2((ndc.x + 1.0) / 2.0, (ndc.y + 1.0) / 2.0);
	int time_trunc = int(Time);
	float time_normalised = Time - float(time_trunc);
	
	vec2 dudvVal = texture(dudvMap, (TexCoord + time_normalised) * WAVE_SCALE).rg * 2.0 - 1.0;
	dudvVal = WAVE_STRENGTH * vec2(dudvVal.x * 2.0, dudvVal.y);
	vec2 reflectionCoord = vec2(ndc.x, -ndc.y) + dudvVal;
	vec2 refractionCoord = vec2(ndc.x, ndc.y) + dudvVal;

	reflectionCoord.y = clamp(reflectionCoord.y, -0.01, -0.9);
	reflectionCoord.x = clamp(reflectionCoord.x, 0.01, 0.9);
	refractionCoord = clamp(refractionCoord, 0.001, 0.999);

	vec4 reflectionColour = texture(reflectionTexture, reflectionCoord);
	vec4 refractionColour = texture(refractionTexture, refractionCoord);
	frag_Colour = mix(reflectionColour, refractionColour, 0.5);
}

