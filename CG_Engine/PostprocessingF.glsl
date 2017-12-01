R"(
in vec2 PassTexCoord;
uniform sampler2D InputImage;

uniform vec2 resolution;

out vec4 outFrag;

vec4 blur(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  vec4 colors[7];
  colors[0] += texture2D(image, uv) * 0.1964825501511404;
  colors[1] += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
  colors[2] += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
  colors[3] += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
  colors[4] += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
  colors[5] += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
  colors[6] += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;
  for(int i =0; i < 7; i++){
	if(colors[i].rgb != vec3(0, 0, 0)){
		color += colors[i];
	}
  }
  return color;
}

void BloomEffect(){
	float brightness = (outFrag.r * 0.2) + (outFrag.g * 0.7) + (outFrag.b * 0.7);

	
		
	
	if(brightness < 0.9){
	;//	return;
	} 

	vec2 iResolution = resolution;
	vec2 uv = vec2(gl_FragCoord.xy / iResolution.xy);
	if(texture(InputImage, uv).rgb == vec3(0,0,0)){
		outFrag = vec4(0,0,0,1);
		return;
	}
	vec4 out_colour = blur(InputImage, uv, iResolution.xy, vec2(0, 1));
	out_colour += 0.5 * blur(InputImage, uv, iResolution.xy, vec2(1, 0));
	
	
	vec4 result = out_colour;
	
	
	if(length(out_colour) > 0.01)
		outFrag += out_colour;
	
	//else
		outFrag = outFrag;
		
}

)"