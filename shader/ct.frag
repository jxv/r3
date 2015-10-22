precision mediump float;

uniform sampler2D u_sample;

varying vec3 v_color;
varying vec2 v_texcoord;

void main() {
	vec3 color = texture2D(u_sample, v_texcoord).xyz * 0.5 + v_color * 0.5;
	gl_FragColor = vec4(color, 1.0);
}
