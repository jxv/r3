#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <GLES2/gl2.h>

#include "../shader/normal.vert.h"
#include "../shader/normal.frag.h"

#include "../shader/cell.vert.h"
#include "../shader/cell.frag.h"

#include "../shader/color.vert.h"
#include "../shader/color.frag.h"

#include "../shader/texture.vert.h"
#include "../shader/texture.frag.h"

#include "../shader/color_normal_texture.vert.h"
#include "../shader/color_normal_texture.frag.h"

void r3_clear(struct r3_ren *ren)
{
	glClearColor(ren->clear_color.x, ren->clear_color.y, ren->clear_color.z, 1);
	glClear(ren->clear_bits);
}

void r3_render(struct r3_ren *ren)
{
	ren->render(ren);
}

void r3_viewport(const struct r3_ren *ren)
{
	glViewport(0, 0, ren->window_size.x, ren->window_size.y);
}

void  r3_enable_tests(const struct r3_ren *ren)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void r3_quit(struct r3_ren *ren)
{
	ren->quit(ren);
}

char *r3_load_file(const char *path)
{
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

char* r3_load_tga(const char *fileName, int *width, int *height)
{
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

unsigned int r3_make_shader(const char *src, GLenum type, int src_len)
{
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

unsigned int r3_load_shader(const char *path, GLenum type)
{
	char *data = r3_load_file(path);
	assert(data != NULL);
	unsigned int result = r3_make_shader(data, type, 0);
	free(data);
	return result;
} 

unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader)
{
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

GLuint r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len)
{
	const unsigned int vert = r3_make_shader(vert_src, GL_VERTEX_SHADER, vert_src_len);
	const unsigned int frag = r3_make_shader(frag_src, GL_FRAGMENT_SHADER, frag_src_len);
	return r3_make_program(vert, frag);
}

GLuint r3_load_program_from_path(const char *vert_path, const char *frag_path)
{
	unsigned int vert = r3_load_shader(vert_path, GL_VERTEX_SHADER);
	unsigned int frag = r3_load_shader(frag_path, GL_FRAGMENT_SHADER);
	return r3_make_program(vert, frag);
}

void r3_set_shader_normal_ids(struct r3_shader *sh)
{
	glUseProgram(sh->program_id);
	// Attrib
	sh->attrib.position_id = glGetAttribLocation(sh->program_id, "a_position");
	sh->attrib.normal_id = glGetAttribLocation(sh->program_id, "a_normal");
	// Uniform
	sh->uniform.mvp_id = glGetUniformLocation(sh->program_id, "u_mvp");
	sh->uniform.normal_id = glGetUniformLocation(sh->program_id, "u_normal");
	sh->uniform.light_position_id = glGetUniformLocation(sh->program_id, "u_light_position");
	sh->uniform.ambient_id = glGetUniformLocation(sh->program_id, "u_ambient");
	sh->uniform.diffuse_id = glGetUniformLocation(sh->program_id, "u_diffuse");
	sh->uniform.specular_id = glGetUniformLocation(sh->program_id, "u_specular");
	sh->uniform.shininess_id = glGetUniformLocation(sh->program_id, "u_shininess");
	sh->uniform.sample_id = glGetUniformLocation(sh->program_id, "u_sample");
}

void r3_set_shader_texture_ids(struct r3_shader *sh)
{
	glUseProgram(sh->program_id);
	// Attrib
	sh->attrib.position_id = glGetAttribLocation(sh->program_id, "a_position");
	sh->attrib.texcoord_id = glGetAttribLocation(sh->program_id, "a_texcoord");
	// Uniform
	sh->uniform.mvp_id = glGetUniformLocation(sh->program_id, "u_mvp");
	sh->uniform.sample_id = glGetUniformLocation(sh->program_id, "u_sample");
}

void r3_set_shader_color_ids(struct r3_shader *sh)
{
	glUseProgram(sh->program_id);
	// Attrib
	sh->attrib.position_id = glGetAttribLocation(sh->program_id, "a_position");
	sh->attrib.color_id = glGetAttribLocation(sh->program_id, "a_color");
	// Uniform
	sh->uniform.mvp_id = glGetUniformLocation(sh->program_id, "u_mvp");
}

void r3_set_shader_color_normal_texture_ids(struct r3_shader *sh)
{
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
	sh->uniform.ambient_id = glGetUniformLocation(sh->program_id, "u_ambient_material");
	sh->uniform.specular_id = glGetUniformLocation(sh->program_id, "u_specular_material");
	sh->uniform.shininess_id = glGetUniformLocation(sh->program_id, "u_shininess");
	sh->uniform.sample_id = glGetUniformLocation(sh->program_id, "u_sample");
}

ssize_t r3_verts_tag_sizeof(enum r3_verts_tag tag)
{
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

ssize_t r3_verts_sizeof(const struct r3_verts *verts)
{
	return verts->len * r3_verts_tag_sizeof(verts->tag);
}

ssize_t r3_indices_tag_sizeof(enum r3_indices_tag tag)
{
	switch (tag) {
	case R3_INDICES_USHORT: return sizeof(unsigned int short);
	case R3_INDICES_UINT: return sizeof(unsigned int);
	}
	assert(false);
}

ssize_t r3_indices_sizeof(const struct r3_indices *indices)
{
	return indices->len * r3_indices_tag_sizeof(indices->tag);
}

void r3_make_mesh_from_spec(const struct r3_spec *spec, struct r3_mesh *m)
{
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, r3_verts_sizeof(&spec->verts), spec->verts.data, GL_STATIC_DRAW);
	glGenBuffers(1, &m->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r3_indices_sizeof(&spec->indices), spec->indices.data, GL_STATIC_DRAW);
	m->num_indices = spec->indices.len;
	m->verts_tag = spec->verts.tag;
}

void r3_render_normal(const struct r3_mesh *m, const struct r3_shader *sh, m4f mv, m4f mvp,
		v3f light_position, v3f ambient, v3f diffuse, v3f specular, float shininess)
{
	glUseProgram(sh->program_id);
	// Set uniforms
	glUniformMatrix4fv(sh->uniform.mvp_id, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh->uniform.normal_id, 1, 0, m3m4f(mv).val);
	glUniform3fv(sh->uniform.light_position_id, 1, light_position.val);
	glUniform3fv(sh->uniform.ambient_id, 1, ambient.val);
	glUniform3fv(sh->uniform.diffuse_id, 1, diffuse.val);
	glUniform3fv(sh->uniform.specular_id, 1, specular.val);
	glUniform1f(sh->uniform.shininess_id, shininess);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glEnableVertexAttribArray(sh->attrib.normal_id);
	glVertexAttribPointer(sh->attrib.normal_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	glDisableVertexAttribArray(sh->attrib.normal_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_texture(const struct r3_mesh *m, const struct r3_shader *sh, unsigned int tex_id, m4f mvp)
{
	glUseProgram(sh->program_id);
	// Set uniforms
	glUniformMatrix4fv(sh->uniform.mvp_id, 1, GL_FALSE, mvp.val);
	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(sh->uniform.sample_id, 0);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glEnableVertexAttribArray(sh->attrib.texcoord_id);
	glVertexAttribPointer(sh->attrib.texcoord_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	glDisableVertexAttribArray(sh->attrib.texcoord_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_color(const struct r3_mesh *m, const struct r3_shader *sh, m4f mvp)
{
	glUseProgram(sh->program_id);
	// Set uniforms
	glUniformMatrix4fv(sh->uniform.mvp_id, 1, GL_FALSE, mvp.val);
	// Set texture
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glEnableVertexAttribArray(sh->attrib.color_id);
	glVertexAttribPointer(sh->attrib.color_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_COLOR));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	glDisableVertexAttribArray(sh->attrib.color_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_color_normal_texture(const struct r3_mesh *m, const struct r3_shader *sh, unsigned int tex_id,
		m4f mv, m4f mvp, v3f light_position, v3f ambient, v3f specular, float shininess)
{
	glUseProgram(sh->program_id);
	// Set uniforms
	glUniformMatrix4fv(sh->uniform.mvp_id, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh->uniform.normal_id, 1, 0, m3m4f(mv).val);
	glUniform3fv(sh->uniform.light_position_id, 1, light_position.val);
	glUniform3fv(sh->uniform.ambient_id, 1, ambient.val);
	glUniform3fv(sh->uniform.specular_id, 1, specular.val);
	glUniform1f(sh->uniform.shininess_id, shininess);
	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(sh->uniform.sample_id, 0);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glEnableVertexAttribArray(sh->attrib.color_id);
	glVertexAttribPointer(sh->attrib.color_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_COLOR));
	glEnableVertexAttribArray(sh->attrib.normal_id);
	glVertexAttribPointer(sh->attrib.normal_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glEnableVertexAttribArray(sh->attrib.texcoord_id);
	glVertexAttribPointer(sh->attrib.texcoord_id, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	glDisableVertexAttribArray(sh->attrib.color_id);
	glDisableVertexAttribArray(sh->attrib.normal_id);
	glDisableVertexAttribArray(sh->attrib.texcoord_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int r3_load_tga_texture(const char *path)
{
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

void r3_make_cell_shader(struct r3_shader *sh)
{
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_cell_vert, shader_cell_vert_len,
		(const char*)shader_cell_frag, shader_cell_frag_len
	);
	r3_set_shader_normal_ids(sh);
}

void r3_make_normal_shader(struct r3_shader *sh)
{
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_normal_vert, shader_normal_vert_len,
		(const char*)shader_normal_frag, shader_normal_frag_len
	);
	r3_set_shader_normal_ids(sh);
}

void r3_make_texture_shader(struct r3_shader *sh)
{
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_texture_vert, shader_texture_vert_len,
		(const char*)shader_texture_frag, shader_texture_frag_len
	);
	r3_set_shader_texture_ids(sh);
}

void r3_make_color_shader(struct r3_shader *sh)
{
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_color_vert, shader_color_vert_len,
		(const char*)shader_color_frag, shader_color_frag_len
	);
	r3_set_shader_color_ids(sh);
}

void r3_make_color_normal_texture_shader(struct r3_shader *sh)
{
	sh->program_id = r3_make_program_from_src(
		(const char*)shader_color_normal_texture_vert, shader_color_normal_texture_vert_len,
		(const char*)shader_color_normal_texture_frag, shader_color_normal_texture_frag_len
	);
	r3_set_shader_color_normal_texture_ids(sh);
}

void r3_break_mesh(const struct r3_mesh *m)
{
	glDeleteBuffers(1, &m->vbo);
	glDeleteBuffers(1, &m->ibo);
}

void r3_break_shader(const struct r3_shader *sh)
{
	glDeleteProgram(sh->program_id);
}

ssize_t r3_offset(enum r3_verts_tag tag, enum r3_vert vert)
{
	switch (tag) {
	case R3_VERTS_PC:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_COLOR:
			return sizeof(v3f);
		default:
			assert(false);
		}
		break;
	case R3_VERTS_PN:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_NORMAL:
			return sizeof(v3f);
		default:
			assert(false);
		}
		break;
	case R3_VERTS_PT:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_TEXCOORD:
			return sizeof(v3f);
		default:
			assert(false);
		}
		break;
	case R3_VERTS_PCN:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_COLOR:
			return sizeof(v3f);
		case R3_NORMAL:
			return sizeof(v3f) + sizeof(v3f);
		default:
			assert(false);
		}
		break;
	case R3_VERTS_PNT:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_NORMAL:
			return sizeof(v3f);
		case R3_TEXCOORD:
			return sizeof(v3f) + sizeof(v3f);
		default:
			assert(false);
		}
		break;
	case R3_VERTS_PCNT:
		switch (vert) {
		case R3_POSITION:
			return 0;
		case R3_COLOR:
			return sizeof(v3f);
		case R3_NORMAL:
			return sizeof(v3f) + sizeof(v3f);
		case R3_TEXCOORD:
			return sizeof(v3f) + sizeof(v3f) + sizeof(v3f);
		}
		break;
	}
	assert(false);
}
