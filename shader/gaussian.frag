precision mediump float;

uniform sampler2D u_sample;
uniform float u_coefficients[3];
uniform vec2 u_offset;

varying vec2 v_texcoord;

void main()
{
	vec3 a = u_coefficients[0] * texture2D(u_sample, v_texcoord - u_offset).xyz;
	vec3 b = u_coefficients[1] * texture2D(u_sample, v_texcoord).xyz;
	vec3 c = u_coefficients[2] * texture2D(u_sample, v_texcoord + u_offset).xyz;
	vec3 color = a + b + c;
	gl_FragColor = vec4(color, 1);
}
