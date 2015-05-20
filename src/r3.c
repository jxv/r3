#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <assert.h>

#include "../shader/normal.vert.h"
#include "../shader/cell.frag.h"
#include "../shader/minimum.frag.h"

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

GLuint r3_make_shader(const char *src, GLenum type, int src_len) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, src_len == 0 ? NULL : &src_len);
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
	unsigned int result = r3_make_shader(data, type, 0);
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

GLuint r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len) {
	const unsigned int vert = r3_make_shader(vert_src, GL_VERTEX_SHADER, vert_src_len);
	const unsigned int frag = r3_make_shader(frag_src, GL_FRAGMENT_SHADER, frag_src_len);
	return r3_make_program(vert, frag);
}

GLuint r3_load_program_from_path(const char *vert_path, const char *frag_path) {
	unsigned int vert = r3_load_shader(vert_path, GL_VERTEX_SHADER);
	unsigned int frag = r3_load_shader(frag_path, GL_FRAGMENT_SHADER);
	return r3_make_program(vert, frag);
}

void r3_set_shader_normal_ids(struct r3_shader *sh) {
	glUseProgram(sh->program_id);
	// Attrib
	sh->attrib.position_id = glGetAttribLocation(sh->program_id, "a_position");
	sh->attrib.normal_id = glGetAttribLocation(sh->program_id, "a_normal");
	sh->attrib.color_id = glGetAttribLocation(sh->program_id, "a_color");
	sh->attrib.texcoord_id = glGetAttribLocation(sh->program_id, "a_texcoord");
	// Uniform
	sh->uniform.mvp_id = glGetUniformLocation(sh->program_id, "u_mvp");
	sh->uniform.normal_id = glGetUniformLocation(sh->program_id, "u_normal");
	sh->uniform.light_position_id = glGetUniformLocation(sh->program_id, "u_light_position");
	sh->uniform.ambient_material_id = glGetUniformLocation(sh->program_id, "u_ambient_material");
	sh->uniform.specular_material_id = glGetUniformLocation(sh->program_id, "u_specular_material");
	sh->uniform.shininess_id = glGetUniformLocation(sh->program_id, "u_shininess");
	sh->uniform.sample_id = glGetUniformLocation(sh->program_id, "u_sample");
}

ssize_t r3_verts_tag_sizeof(enum r3_verts_tag tag) {
	switch (tag) {
	case R3_VERTS_PC: return sizeof(struct r3_pc);
	case R3_VERTS_PN: return sizeof(struct r3_pn);
	case R3_VERTS_PCN: return sizeof(struct r3_pcn);
	case R3_VERTS_PT: return sizeof(struct r3_pt);
	case R3_VERTS_PNT: return sizeof(struct r3_pnt);
	case R3_VERTS_PCNT: return sizeof(struct r3_pcnt);
	}
	assert(false);
}

ssize_t r3_verts_sizeof(const struct r3_verts *verts) {
	return verts->len * r3_verts_tag_sizeof(verts->tag);
}

ssize_t r3_indices_tag_sizeof(enum r3_indices_tag tag) {
	switch (tag) {
	case R3_INDICES_USHORT: return sizeof(unsigned int short);
	case R3_INDICES_UINT: return sizeof(unsigned int);
	}
	assert(false);
}

ssize_t r3_indices_sizeof(const struct r3_indices *indices) {
	return indices->len * r3_indices_tag_sizeof(indices->tag);
}

void r3_make_mesh_from_spec(const struct r3_spec *spec, struct r3_mesh *m) {
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, r3_verts_sizeof(&spec->verts), spec->verts.data, GL_STATIC_DRAW);
	glGenBuffers(1, &m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r3_indices_sizeof(&spec->indices), spec->indices.data, GL_STATIC_DRAW);
	m->num_indices = spec->indices.len;
	m->verts_tag = spec->verts.tag;
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
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
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
			.texcoord = _v2f(texcoords[i*2+0], texcoords[i*2+1]),
		};
	}

	memcpy(spec->indices.data, indices, sizeof(unsigned int short) * 36);

	return spec;
}

void r3_render_resource(const struct r3_resource *r,
			m4f mv, m4f mvp, v3f light_position, v3f ambient_material, v3f specular_material, float shininess) {
	assert(r->mesh.verts_tag == R3_VERTS_PCNT);
	glUseProgram(r->shader.program_id);
	const struct r3_shader *sh = &r->shader;
	// Set uniforms
	glUniformMatrix4fv(sh->uniform.mvp_id, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh->uniform.normal_id, 1, 0, m3fm4f(mv).val);
	glUniform3fv(sh->uniform.light_position_id, 1, _v3f(0.25, 0.25, 1).val);
	glUniform3f(sh->uniform.ambient_material_id, 0.05, 0.05, 0.05);
	glUniform3f(sh->uniform.specular_material_id, 0.5, 0.5, 0.5);
	glUniform1f(sh->uniform.shininess_id, 100);
	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, r->tex_id);
	glUniform1i(sh->uniform.sample_id, 0);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, r->mesh.vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), NULL);
	glEnableVertexAttribArray(sh->attrib.color_id);
	glVertexAttribPointer(sh->attrib.color_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)sizeof(v3f));
	glEnableVertexAttribArray(sh->attrib.normal_id);
	glVertexAttribPointer(sh->attrib.normal_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)(sizeof(v3f) * 2));
	glEnableVertexAttribArray(sh->attrib.texcoord_id);
	glVertexAttribPointer(sh->attrib.texcoord_id, 3, GL_FLOAT, GL_FALSE, sizeof(struct r3_pcnt), (void*)(sizeof(v3f) * 3));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->mesh.ibo);
	glDrawElements(GL_TRIANGLES, r->mesh.num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	glDisableVertexAttribArray(sh->attrib.color_id);
	glDisableVertexAttribArray(sh->attrib.normal_id);
	glDisableVertexAttribArray(sh->attrib.texcoord_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int r3_load_tga_texture(const char *path) {
	int width, height;
	char *data = r3_load_tga(path, &width, &height);
	if (data) {
		unsigned int tex_id;
		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		free(data);
		return tex_id;
	}
	return 0;
}

void r3_make_normal_shader(struct r3_shader *sh) {
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_normal_vert, shader_normal_vert_len,
		(const char*)shader_minimum_frag, shader_minimum_frag_len
	);
	r3_set_shader_normal_ids(sh);
}

void r3_break_mesh(const struct r3_mesh *m) {
	glDeleteBuffers(1, &m->vbo);
	glDeleteBuffers(1, &m->ibo);
}

void r3_break_shader(const struct r3_shader *sh) {
	glDeleteProgram(sh->program_id);
}
