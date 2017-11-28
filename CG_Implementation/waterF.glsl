#version 330

in vec4 ClipspaceCoord;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

out vec4 frag_Colour;

void main(){
	vec2 ndc = ClipspaceCoord.xy/ClipspaceCoord.w;
	ndc = vec2((ndc.x + 1.0) / 2.0, (ndc.y + 1.0) / 2.0);

	vec2 reflectionCoord = vec2(ndc.x, -1.0 * ndc.y);
	vec2 refractionCoord = vec2(ndc.x, ndc.y);

	vec4 reflectionColour = texture(reflectionTexture, reflectionCoord);
	vec4 refractionColour = texture(refractionTexture, refractionCoord);

	frag_Colour = mix(reflectionColour, refractionColour, 0.5);
}

