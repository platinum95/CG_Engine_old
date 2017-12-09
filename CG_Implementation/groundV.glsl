#version 330 core

layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
  vec4 ClippingPlane;
};
layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

in vec2 MeshXZ;
in float Height;

uniform float GroundTranslation;


void main(){
	vec4 vPos = vec4(MeshXZ.x, Height, MeshXZ.y, 1.0);

	vec4 WorldPosition = GroundTranslation * vPos;
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);

	gl_Position = PV_Matrix * WorldPosition;
}
