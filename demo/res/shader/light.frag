precision mediump float;

uniform vec3 u_light_position;
uniform vec3 u_ambient_material;
uniform vec3 u_diffuse_material;
uniform vec3 u_specular_material;
uniform float u_shininess;

varying vec3 v_eyespace_normal;

void main() {
	vec3 n = normalize(v_eyespace_normal);
	vec3 l = normalize(u_light_position);
	vec3 e = vec3(0, 0, 1);
	vec3 h = normalize(l + e);

	float df = max(0, dot(n, l));
	float sf = max(0, dot(n, h));
	sf = pow(sf, u_shininess);

	vec3 color = u_ambient_material + df * u_diffuse_material + sf * u_specular_material;

	gl_FragColor = vec4(color, 1);
}
