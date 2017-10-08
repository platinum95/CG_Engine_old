#version 330                                                                        
out vec4 FragColor;
in vec4 PassColour;

float CONV(int a){
	int mod = a%360;
	float ratio = 3.14/180.0;

	return mod * ratio;
}

float normalise(float val){
	return (val +1.0)/ 2.0;
}
void main(){
	float x = CONV(int(gl_FragCoord.x));
	float y = CONV(int(gl_FragCoord.y));
	FragColor = PassColour;//vec4(normalise(sin(x)), normalise(sin(x)), normalise(sin(x)), 1.0);//PassColour;
}