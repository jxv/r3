precision mediump float;

uniform vec3 u_light_position;
uniform vec3 u_ambient;
uniform vec3 u_specular;
uniform float u_shininess;

varying vec3 v_color;
varying vec3 v_eyespace_normal;

void main() {
	vec3 n = normalize(v_eyespace_normal);
	vec3 l = normalize(u_light_position);
	vec3 e = vec3(0, 0, 1);
	vec3 h = normalize(l + e);

	float df = max(0.0, dot(n, l));
	float sf = max(0.0, dot(n, h));
	sf = pow(sf, u_shininess);
	if (df < 0.3) {
		df = 0.3;
	}

	sf = step(0.5, sf);

	vec3 color = u_ambient + df * v_color + sf * u_specular;
	gl_FragColor = vec4(color, 1);
}
