uniform mat4 u_mvp;
uniform mat3 u_normal;

attribute vec4 a_position;
attribute vec3 a_normal;

varying vec3 v_eyespace_normal;

void main() {
    v_eyespace_normal = u_normal * a_normal;
    gl_Position = u_mvp * a_position;
}
