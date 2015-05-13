#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <GLES2/gl2.h>
#include <assert.h>

char *load_file(const char *path) {
	FILE *file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	rewind(file);
	char *data = calloc(length + 1, sizeof(char));
	if (fread(data, 1, length, file) == 0) {
		free(data);
		return NULL;
	}
	fclose(file);
	return data;
}

unsigned int make_shader(GLenum type, const char *source) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char *info = calloc(length, sizeof(GLchar));
		glGetShaderInfoLog(shader, length, NULL, info);
		fprintf(stderr, "glCompileShader failed:\n%s\n", info);
		free(info);
	}
	return shader;
}

unsigned int load_shader(const char *path, GLenum type) {
	char *data = load_file(path);
	assert(data != NULL);
	unsigned int result = make_shader(type, data);
	free(data);
	return result;
}

unsigned int make_program(unsigned int vert_shader, unsigned int frag_shader) {
	unsigned int program_id = glCreateProgram();
	glAttachShader(program_id, vert_shader);
	glAttachShader(program_id, frag_shader);
	glLinkProgram(program_id);
	int status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	if (status == false) {
		int length;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
		char *info = calloc(length, sizeof(char));
		glGetProgramInfoLog(program_id, length, NULL, info);
		fprintf(stderr, "glLinkProgram failed: %s\n", info);
		free(info);
	}
	glDetachShader(program_id, vert_shader);
	glDetachShader(program_id, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	return program_id;
}

unsigned int load_program(const char *vert_path, const char *frag_path) {
	unsigned int vert = load_shader(vert_path, GL_VERTEX_SHADER);
	unsigned int frag = load_shader(frag_path, GL_FRAGMENT_SHADER);
	return make_program(vert, frag);
}

static const float box_positions[4 * 6][3] = {
	{ 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}, { 1, 1,-1},
	{ 1, 1,-1}, {-1, 1, 1}, {-1, 1, 1}, { 1, 1, 1},
	{-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1},
	{-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1},
	{ 1,-1,-1}, { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1},
	{ 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1},
};

static const float box_normals[4 * 6][3] = {
	{ 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0},
	{ 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0},
	{ 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1},
	{-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
	{ 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0},
	{ 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1},
};

static const float box_uvs[4 * 6][2] = {
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
	{0,0}, {0,1}, {1,1}, {1,0},
};

struct r3_mesh_spec r3_box_ctor(v3f dimen) {
	const v3f radii = divv3fs(dimen, 2);
	struct r3_mesh_spec spec;

	spec.num_points = 4 * 6;
	spec.points = malloc(sizeof(struct r3_mesh_point) * spec.num_points);

	for (int i = 0; i < spec.num_points; i++) {
		const v3f position = mulv3f(*(v3f*)box_positions[i], radii);
		spec.points[i] = (struct r3_mesh_point) {
			.position = position,
			.color = divv3f(addv3f(position, radii), dimen),
			.normal = *(v3f*)box_normals[i],
			.uv = *(v2f*)box_uvs[i],
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
		.pvm_mat_id = glGetAttribLocation(program_id, "pvmMatrix"),
		.vm_mat_id = glGetAttribLocation(program_id, "viewModelMatrix"),
		.normal_mat_id = glGetAttribLocation(program_id,
						     "normalMatrix"),
		.diffuse_color_id = glGetAttribLocation(program_id,
							"diffuseColor"),
		.ambient_color_id = glGetAttribLocation(program_id,
							"ambientColor"),
		.specular_color_id = glGetAttribLocation(program_id,
							 "specularColor"),
		.shininess_id = glGetAttribLocation(program_id,
						    "shininess"),
		.light_direction_id = glGetAttribLocation(program_id,
							  "lightDirection"),
		.diffuse_map_id = glGetAttribLocation(program_id,
						      "diffuseMap"),
	};
	rsrc->shader = shader;
}
