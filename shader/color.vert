attribute vec4 a_position;
attribute vec3 a_color;

uniform mat4 u_mvp;

varying vec3 v_color;

void main() {
	v_color = a_color;
	gl_Position = u_mvp * a_position;
}
