#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <assert.h>

void r3_clear(struct r3_ren *ren) {
	glClearColor(ren->clear_color.x, ren->clear_color.y, ren->clear_color.z, 1);
	glClear(ren->clear_bits);
}

void r3_render(struct r3_ren *ren) {
	ren->render(ren);
}

void r3_viewport(const struct r3_ren *ren) {
	glViewport(0, 0, ren->window_size.x, ren->window_size.y);
}

void  r3_enable_tests(const struct r3_ren *ren) {
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void r3_quit(struct r3_ren *ren) {
	ren->quit(ren);
}

char *r3_load_file(const char *path) {
	FILE *file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	int len = ftell(file);
	rewind(file);
	char *data = calloc(len + 1, sizeof(char));
	if (fread(data, 1, len, file) == 0) {
		free(data);
		return NULL;
	}
	fclose(file);
	return data;
}

char* r3_load_tga(const char *fileName, int *width, int *height) {
	char *buf = NULL;
	FILE *f;
	unsigned char tgaheader[12];
	unsigned char attributes[6];
	unsigned int imagesize;
	f = fopen(fileName, "rb");
	if(f == NULL) return NULL;

	if (fread(&tgaheader, sizeof(tgaheader), 1, f) == 0) {
		fclose(f);
		return NULL;
	}
	if (fread(attributes, sizeof(attributes), 1, f) == 0) {
		fclose(f);
		return 0;
	}
	*width = attributes[1] * 256 + attributes[0];
	*height = attributes[3] * 256 + attributes[2];
	imagesize = attributes[4] / 8 * *width * *height;
	buf = malloc(imagesize);
	if (buf == NULL) {
		fclose(f);
		return 0;
	}
	if (fread(buf, 1, imagesize, f) != imagesize) {
		free(buf);
		return NULL;
	}
	fclose(f);
	return buf;
}

GLuint r3_make_shader(const char *source, GLenum type) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char *info = calloc(len, sizeof(GLchar));
		glGetShaderInfoLog(shader, len, NULL, info);
		fprintf(stderr, "glCompileShader failed:\n%s\n", info);
		free(info);
	}
	return shader;
}

GLuint r3_load_shader(const char *path, GLenum type) {
	char *data = r3_load_file(path);
	assert(data != NULL);
	unsigned int result = r3_make_shader(data, type);
	free(data);
	return result;
} 

GLuint r3_make_program(unsigned int vert_shader, unsigned int frag_shader) {
	unsigned int program_id = glCreateProgram();
	glAttachShader(program_id, vert_shader);
	glAttachShader(program_id, frag_shader);
	glLinkProgram(program_id);
	int status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	if (status == false) {
		int len;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len);
		char *info = calloc(len, sizeof(char));
		glGetProgramInfoLog(program_id, len, NULL, info);
		fprintf(stderr, "glLinkProgram failed: %s\n", info);
		free(info);
	}
	glDetachShader(program_id, vert_shader);
	glDetachShader(program_id, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	return program_id;
}

GLuint r3_make_program_from_src(const char *vert_src, const char *frag_src) {
	const unsigned int vert = r3_make_shader(vert_src, GL_VERTEX_SHADER);
	const unsigned int frag = r3_make_shader(frag_src, GL_FRAGMENT_SHADER);
	return r3_make_program(vert, frag);
}

GLuint r3_load_program_from_path(const char *vert_path, const char *frag_path) {
	unsigned int vert = r3_load_shader(vert_path, GL_VERTEX_SHADER);
	unsigned int frag = r3_load_shader(frag_path, GL_FRAGMENT_SHADER);
	return r3_make_program(vert, frag);
}

ssize_t r3_verts_sizeof(const struct r3_verts *verts) {
	switch (verts->tag) {
	case R3_VERTS_PC: return verts->len * sizeof(struct r3_pc);
	case R3_VERTS_PN: return verts->len * sizeof(struct r3_pn);
	case R3_VERTS_PCN: return verts->len * sizeof(struct r3_pcn);
	case R3_VERTS_PT: return verts->len * sizeof(struct r3_pt);
	case R3_VERTS_PNT: return verts->len * sizeof(struct r3_pnt);
	case R3_VERTS_PCNT: return verts->len * sizeof(struct r3_pcnt);
	}
	assert(false);
}

ssize_t r3_indices_sizeof(const struct r3_indices *indices) {
	switch (indices->tag) {
	case R3_INDICES_USHORT: return indices->len * sizeof(unsigned int short);
	}
	assert(false);
}

void r3_make_mesh_from_spec(const struct r3_spec *spec, struct r3_mesh *m) {
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, r3_verts_sizeof(&spec->verts), spec->verts.data, GL_STATIC_DRAW);
	glGenBuffers(1, &m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r3_indices_sizeof(&spec->indices), spec->indices.data, GL_STATIC_DRAW);
	m->num_indices = spec->indices.len;
}

/*
static const float cuboid_positions[4 * 6][3] = {
	{ 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}, { 1, 1,-1},
	{ 1, 1,-1}, {-1, 1, 1}, {-1, 1, 1}, { 1, 1, 1},
	{-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1},
	{-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1},
	{ 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1},
	{ 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1},
};

static const float cuboid_normals[4 * 6][3] = {
	{ 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0},
	{ 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0},
	{ 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1},
	{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
	{ 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0},
	{ 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1},
};

static const float cuboid_uvs[4 * 6][2] = {
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
};

struct r3_mesh_spec r3_cuboid_ctor(v3f dimen) {
	const v3f radii = divv3fs(dimen, 2);
	struct r3_mesh_spec spec;

	spec.num_points = 4 * 6;
	spec.points = malloc(sizeof(struct r3_mesh_point) * spec.num_points);

	for (int i = 0; i < spec.num_points; i++) {
		const v3f position = mulv3f(*(v3f*)cuboid_positions[i], radii);
		spec.points[i] = (struct r3_mesh_point) {
			.position = position,
			.color = divv3f(addv3f(position, radii), dimen),
			.normal = *(v3f*)cuboid_normals[i],
			.uv = *(v2f*)cuboid_uvs[i],
		};
	}

	spec.num_indices = 12;
	spec.indices = malloc(sizeof(v3ui) * spec.num_indices);
	for (int i = 0, j = 0; i < 6; i++, j += 2) {
		const int a = 0 + i * 4;
		const int b = 1 + i * 4;
		const int c = 2 + i * 4;
		const int d = 3 + i * 4;
		spec.indices[j + 0] = _v3ui(a, b, c);
		spec.indices[j + 1] = _v3ui(c, d, a);
	}

	return spec;
}

void r3_initialize(const char *path, struct r3_resource *rsrc) {
	const int width = 0;
	const int height = 0;
	void *data = NULL;

	glGenTextures(1, &rsrc->texture_id);
	glBindTexture(GL_TEXTURE_2D, rsrc->texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	const unsigned int program_id = load_program("shader.vert",
						     "shader.frag");
	const struct r3_shader shader = (struct r3_shader) {
		.program_id = program_id,
		.position_id = glGetAttribLocation(program_id, "position"),
		.color_id = glGetAttribLocation(program_id, "color"),
		.normal_id = glGetAttribLocation(program_id, "normal"),
		.uv_id = glGetAttribLocation(program_id, "uv"),
		.pvm_mat_id = glGetUniformLocation(program_id, "pvmMatrix"),
		.vm_mat_id = glGetUniformLocation(program_id, "viewModelMatrix"),
		.normal_mat_id = glGetUniformLocation(program_id,
						      "normalMatrix"),
		.diffuse_color_id = glGetUniformLocation(program_id,
							 "diffuseColor"),
		.ambient_color_id = glGetUniformLocation(program_id,
							 "ambientColor"),
		.specular_color_id = glGetUniformLocation(program_id,
							  "specularColor"),
		.shininess_id = glGetUniformLocation(program_id,
						     "shininess"),
		.light_direction_id = glGetUniformLocation(program_id,
							   "lightDirection"),
		.diffuse_map_id = glGetUniformLocation(program_id,
						       "diffuseMap"),
	};
	rsrc->shader = shader;
	
	glGenBuffers(1, &rsrc->mesh.vbo);
	glGenBuffers(1, &rsrc->mesh.ibo);

	const struct r3_mesh_spec spec = r3_cuboid_ctor(_v3f(1,1,1));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rsrc->mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(*spec.points) * spec.num_points, spec.points, GL_STATIC_DRAW);
	//const int index_buf_size = sizeof(indices) * v;
	//const int index_buf_size = sizeof(*indices) * v;
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buf_size, is_ptr, GL_STATIC_DRAW);
}
*/
