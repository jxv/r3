precision lowp float;

uniform vec3 u_kolor;

void main() {
	gl_FragColor = vec4(u_kolor, 1.0);
}
