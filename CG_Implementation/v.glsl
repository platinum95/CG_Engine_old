#version 330
                                                              
layout (location = 0) in vec3 vPosition ;															
in vec4 fColor;																
in float time;								
out vec4 color;														
#define SCALE_FACTOR 0.5												
mat4 scale = mat4(  SCALE_FACTOR, 0, 0, 0,								
					0, SCALE_FACTOR, 0 ,0,									
					0, 0, SCALE_FACTOR, 0,							
					0, 0, 0, 1   );										
                                                                      
void main()                                                                
{                                                     
	gl_PointSize = 10.0;
    gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0); 
	//float offsdet = (sin(time) + 1.0) / 2.0;							
	//float offsetb = (cos(time * 2) + 1.0) / 2.0;							
	//color = fColor;//vec4(vColor.r / offset, vColor.g * offsetb, offset +vColor.b * offset, vColor.a);					
}

