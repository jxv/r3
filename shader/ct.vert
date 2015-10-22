uniform mat4 u_mvp;

attribute vec4 a_position;
attribute vec3 a_color;
attribute vec2 a_texcoord;

varying vec3 v_color;
varying vec2 v_texcoord;

void main() {
	v_color = a_color;
	v_texcoord = a_texcoord;
	gl_Position = u_mvp * a_position;
}
