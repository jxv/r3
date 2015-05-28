precision mediump float;

uniform sampler2D u_sample;
uniform float u_threshold;

varying vec2 v_texcoord;

const vec3 perception = vec3(0.299, 0.587, 0.114);

void main() {
	vec3 color = texture2D(u_sample, v_texcoord).xyz;
	float luminance = dot(perception, color);
	gl_FragColor = (luminance > u_threshold) ? vec4(color, 1) : vec4(0);
}
