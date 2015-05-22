uniform mat4 u_mvp;
uniform mat3 u_normal;

attribute vec4 a_position;
attribute vec3 a_color;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

varying vec3 v_color;
varying vec3 v_eyespace_normal;
varying vec2 v_texcoord;

void main() {
	v_color = a_color;
	v_eyespace_normal = u_normal * a_normal;
	v_texcoord = a_texcoord;
	gl_Position = u_mvp * a_position;
}
