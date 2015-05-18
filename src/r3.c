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

struct r3_spec *r3_create_cuboid_spec() {
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

	char *buf = malloc(sizeof(struct r3_spec) + 24 * sizeof(struct r3_pcnt) + 36 * sizeof(unsigned short int));
	struct r3_spec *spec = (void*)buf;
	spec->verts.tag = R3_VERTS_PCNT;
	spec->verts.len = 24;
	spec->verts.data = (void*) (sizeof(struct r3_spec) + buf);
	spec->indices.tag = R3_INDICES_USHORT;
	spec->indices.len = 36;
	spec->indices.data = (void*)(24 * sizeof(struct r3_pcnt) + sizeof(struct r3_spec) + buf);

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
