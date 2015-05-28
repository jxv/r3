precision mediump float;

uniform sampler2D u_sample;
uniform float u_alpha;

varying vec2 v_texcoord;

void main() {
	vec3 color = texture2D(u_sample, v_texcoord).xyz;
	gl_FragColor = vec4(color * u_alpha, 1);
}
