R"(
#version 330 core

layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
};

in vec3 Velocity;
in float Time;
in float Size;
in vec3 Colour;
in float Opacity;
in float Lifetime;

uniform float CurrentTime;
uniform vec3 EmitterPosition;
uniform vec3 EmitterDirection;
uniform mat4 model;

out float PassTime;
out vec3 PassColour;
out float PassOpacity;


void main(){
	PassColour = Colour;
	float elapsed_time = CurrentTime - Time;
	elapsed_time = mod (elapsed_time, 5.0);
	PassOpacity = 1.0;
	float lifetime_decay = Lifetime / 3.5;
	if(elapsed_time > lifetime_decay){
		float decay_time = Lifetime - elapsed_time;
		PassOpacity = max(decay_time/lifetime_decay, 0);
	}
	PassTime = elapsed_time;
	vec3 ePos = vec3(0, 0, 0);//vec3(emitter_position.xyz);
	vec3 a = vec3(0, -1, 1);
	vec3 velocity = Velocity + EmitterDirection;
	ePos += velocity * elapsed_time + 0.5 * a * elapsed_time * elapsed_time;
	gl_Position = PV_Matrix * model * vec4(ePos, 1);
	gl_PointSize = Size;

}
)"
