#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ml.h>
#include <r3_sdl.h>

#ifdef GCW0
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
#else
#define WINDOW_WIDTH (320*2)
#define WINDOW_HEIGHT (240*2)
#endif

struct r3_ren ren;

#define	R3_ATTRIB_NORMAL 0x01
#define R3_ATTRIB_TEXCOORD 0x02
#define R3_ATTRIB_COLOR 0x04

struct attrib {
	unsigned char bits;
	int position_id; // always
	int normal_id;
	int texcoord_id;
	int color_id;
};

#define R3_UNIFORM_NORMAL 0x01
#define R3_UNIFORM_LIGHT_POSITION 0x02
#define R3_UNIFORM_AMBIENT_MATERIAL 0x04
#define R3_UNIFORM_SPECULAR_MATERIAL 0x04
#define R3_UNIFORM_SHININESS 0x08
#define R3_UNIFORM_SAMPLE 0x10

struct uniform {
	unsigned char bits;
	int mvp_id;	// always
	int normal_id;
	int light_position_id;
	int ambient_material_id;
	int specular_material_id;
	int shininess_id;
	int sample_id;
};

struct shader {
	struct attrib attrib;
	struct uniform uniform;
	int texcoord_id;
};


struct attrib attrib;
struct uniform uniform;

unsigned int program_id;
int tex_id;
float *verts;
unsigned int *indices;
unsigned int num_indices;
float angle = 0;
char *img_data = NULL;
int img_width, img_height;
m4f mv;
m4f mvp;
struct r3_mesh mesh;

const float dt = 1 / 60.0;
const float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

struct r3_spec *create_cuboid_spec();

const char *vsh =  
"uniform mat4 u_mvp;\n"
"uniform mat3 u_normal;\n"
"\n"
"attribute vec4 a_position;\n"
"attribute vec3 a_color;\n"
"attribute vec3 a_normal;\n"
"attribute vec2 a_texcoord;\n"
"\n"
"varying vec3 v_color;\n"
"varying vec3 v_eyespace_normal;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	v_color = a_color;\n"
"	v_eyespace_normal = u_normal * a_normal;\n"
"	v_texcoord = a_texcoord;\n"
"	gl_Position = u_mvp * a_position;\n"
"}\n";

const char *fsh =  
"precision mediump float;\n"
"\n"
"uniform vec3 u_light_position;\n"
"uniform vec3 u_ambient_material;\n"
"uniform vec3 u_specular_material;\n"
"uniform sampler2D u_sample;\n"
"uniform float u_shininess;\n"
"\n"
"varying vec3 v_color;\n"
"varying vec3 v_eyespace_normal;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	vec3 n = normalize(v_eyespace_normal);\n"
"	vec3 l = normalize(u_light_position);\n"
"	vec3 e = vec3(0, 0, 1);\n"
"	vec3 h = normalize(l + e);\n"
"\n"
"	float df = max(0.0, dot(n, l));\n"
"	float sf = max(0.0, dot(n, h));\n"
"	sf = pow(sf, u_shininess);\n"
"\n"

// No cell shading
/*
"	if (df < 0.1) df = 0.1;\n"
"	else if (df < 0.3) df = 0.3;\n"
"	else if (df < 0.6) df = 0.6;\n"
"	else df = 1.0;\n"
*/

// Minimum value
"	if (df < 0.3) df = 0.3;\n"

"\n"
"	sf = step(0.5, sf);\n"
"\n"
"	vec3 color = u_ambient_material + df * v_color + sf * u_specular_material;\n"
"	gl_FragColor = vec4(color, 1.0) * 0.8 + texture2D(u_sample, v_texcoord) * 0.2;\n"
"}\n";

int main(int argc, char *argv[]) {
	// Init
	r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
	r3_viewport(&ren);
	r3_enable_tests(&ren);
	ren.clear_color = _v3f(0.2, 0.2, 0.2);

	// Setup cube
	program_id = r3_make_program_from_src(vsh, fsh);
	glUseProgram(program_id);
	attrib.position_id = glGetAttribLocation(program_id, "a_position");
	attrib.color_id = glGetAttribLocation(program_id, "a_color");
	attrib.normal_id = glGetAttribLocation(program_id, "a_normal");
	attrib.texcoord_id = glGetAttribLocation(program_id, "a_texcoord");
	uniform.mvp_id = glGetUniformLocation(program_id, "u_mvp");
	uniform.normal_id = glGetUniformLocation(program_id, "u_normal");
	uniform.light_position_id = glGetUniformLocation(program_id, "u_light_position");
	uniform.ambient_material_id = glGetUniformLocation(program_id, "u_ambient_material");
	uniform.specular_material_id = glGetUniformLocation(program_id, "u_specular_material");
	uniform.shininess_id = glGetUniformLocation(program_id, "u_shininess");
	uniform.sample_id = glGetUniformLocation(program_id, "u_sample");

	{
		struct r3_spec *spec = create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &mesh);
		free(spec);
	}

	img_data = r3_load_tga("res/img/base_map.tga", &img_width, &img_height);
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	free(img_data);

/*
	printf("texid:%d, mvp:%d, normal:%d, lightpos:%d, ambient:%d, specular:%d, shininess:%d, sample:%d\n",
		tex_id,
		uniform.mvp_id, uniform.normal_id, uniform.light_position_id, uniform.ambient_material_id,
		uniform.specular_material_id, uniform.shininess_id, uniform.sample_id
	);
*/

	bool done = false;
	while (!done) {
		// Input
		{
			SDL_Event event;
			while (SDL_PollEvent(&event) && !done) {
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
				}
			}
		}
		// Update
		{
			angle = fmodf(angle + dt * 2, M_PI * 2);
			const m4f persp = perspm4f(45, aspect, 1, 20);
			const m4f translate = translatem4f(_v3f(0,0,-7));
			const m4f rot = rotm4f(angle, _v3f(0.9,0.5,0.2));
			mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
			mvp = mulm4f(persp, mv);
		}
		// Render
		{
			r3_viewport(&ren);
			r3_clear(&ren);

			// 
			glUseProgram(program_id);

			// Set uniforms
			glUniformMatrix4fv(uniform.mvp_id, 1, GL_FALSE, mvp.val);
        		glUniformMatrix3fv(uniform.normal_id, 1, 0, m3fm4f(mv).val);
        		glUniform3fv(uniform.light_position_id, 1, _v3f(0.25, 0.25, 1).val);
			glUniform3f(uniform.ambient_material_id, 0.05, 0.05, 0.05);
			glUniform3f(uniform.specular_material_id, 0.5, 0.5, 0.5);
			glUniform1f(uniform.shininess_id, 100);

			// 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glUniform1i(uniform.sample_id, 0);

			// VBO & IBO 
			glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
			glEnableVertexAttribArray(attrib.position_id);
			glEnableVertexAttribArray(attrib.color_id);
			glEnableVertexAttribArray(attrib.normal_id);
			glEnableVertexAttribArray(attrib.texcoord_id);
			glVertexAttribPointer(attrib.position_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), NULL);
			glVertexAttribPointer(attrib.color_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)sizeof(v3f));
			glVertexAttribPointer(attrib.normal_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)(sizeof(v3f) * 2));
			glVertexAttribPointer(attrib.texcoord_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)(sizeof(v3f) * 3));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
			glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_SHORT, NULL);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(attrib.position_id);
			glDisableVertexAttribArray(attrib.color_id);
			glDisableVertexAttribArray(attrib.normal_id);
			glDisableVertexAttribArray(attrib.texcoord_id);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			//

			//
			r3_render(&ren);
		}

		SDL_Delay(16);
	}
	// Clean up
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteProgram(program_id);
	r3_quit(&ren);
	return EXIT_SUCCESS;
}

struct r3_spec *create_cuboid_spec() {
	const float colors[3*24] = {
		// Red
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		// Cyan
		0.0, 1.0, 1.0,
		0.0, 1.0, 1.0,
		0.0, 1.0, 1.0,
		0.0, 1.0, 1.0,
		// Green
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		// Magenta
		1.0, 0.0, 1.0,
		1.0, 0.0, 1.0,
		1.0, 0.0, 1.0,
		1.0, 0.0, 1.0,
		// Blue
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,
		// Yellow
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
		1.0, 1.0, 0.0,
	};
	const float positions[3*24] = {
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f,  0.5f, 0.5f,
		0.5f,  0.5f, 0.5f, 
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
	};
	const float normals[3*24] = {
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};
	const float texcoords[2*24] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
	};
	const unsigned short int indices[36] = {
		0, 2, 1,
		0, 3, 2, 
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11, 
		12, 15, 14,
		12, 14, 13, 
		16, 17, 18,
		16, 18, 19, 
		20, 23, 22,
		20, 22, 21
	};

	void *buf = malloc(sizeof(struct r3_spec) + 24 * sizeof(struct r3_pcnt) + 36 * sizeof(unsigned short int));
	struct r3_spec *spec = buf;
	spec->verts.tag = R3_VERTS_PCNT;
	spec->verts.len = 24;
	spec->verts.data = sizeof(struct r3_spec) + buf;
	spec->indices.tag = R3_INDICES_USHORT;
	spec->indices.len = 36;
	spec->indices.data = 24 * sizeof(struct r3_pcnt) + sizeof(struct r3_spec) + buf;

	for (int i = 0; i < 24; i++) {
		spec->verts.pcnt[i] = (struct r3_pcnt) {
			.position = _v3f(positions[i*3+0], positions[i*3+1], positions[i*3+2]),
			.color = _v3f(colors[i*3+0], colors[i*3+1], colors[i*3+2]),
			.normal = _v3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]),
			.uv = _v2f(texcoords[i*2+0], texcoords[i*2+1]),
		};
	}

	memcpy(spec->indices.data, indices, sizeof(unsigned int short) * 36);

	return spec;
}
