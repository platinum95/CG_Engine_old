#version 330

layout (location = 0) in vec3 vPosition;
#define SCALE_FACTOR 0.5
mat4 scale = mat4(  SCALE_FACTOR, 0, 0, 0,
					0, SCALE_FACTOR, 0 ,0,
					0, 0, SCALE_FACTOR, 0,
					0, 0, 0, 1   );

void main(){
    gl_Position = scale * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
}