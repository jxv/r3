precision mediump float;

uniform sampler2D u_sample;
uniform float u_coefficients[3];
uniform vec2 u_offset;

float offset[5];
float weight[5];

varying vec2 v_texcoord;

void main()
{
	offset[0] = 0.0; offset[1] = 1.0; offset[2] = 2.0; offset[3] = 3.0; offset[4] = 4.0;
	weight[0] = 0.2270270270; weight[1] = 0.1945945946; weight[2] = 0.1216216216;
	weight[3] = 0.0540540541; weight[4] = 0.0162162162;
	
	gl_FragColor = texture2D(u_sample, v_texcoord) * weight[0];
	for (int i = 1; i < 5; i++) {
		gl_FragColor += texture2D(u_sample, (v_texcoord + vec2(0.0, offset[i]))) * weight[i];
		gl_FragColor += texture2D(u_sample, (v_texcoord - vec2(0.0, offset[i]))) * weight[i];
	}
}
