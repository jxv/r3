/* Beware! Rampant macro usage! */

#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "r3_private.h"

SDL_Window *window;
v2i window_dim = {.x = 0, .y = 0};
SDL_GLContext context;
struct r3_mesh mesh_cube, mesh_quad;

static bool r3_sdl_init_video()
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize ren driver: %s\n", SDL_GetError());
		return false;
    }
	return true;
} 
static void r3_sdl_set_gl_attributes()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
}

static SDL_Window *r3_sdl_create_window(const char *title, int width, int height)
{
	return SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL
	);
}

static bool r3_sdl_create_gl_context()
{
	context = SDL_GL_CreateContext(window);
	if (!context) {
	  SDL_Log("SDL_GL_CreateContext(): %s\n", SDL_GetError());
	  return false;
	}
	SDL_GL_SetSwapInterval(1); // VSYNC
	int status = SDL_GL_MakeCurrent(window, context);
	if (status) {
		SDL_Log("SDL_GL_MakeCurrent(): %s\n", SDL_GetError());
		return false;
	}
	return true;
}

void r3_render()
{
    SDL_GL_SwapWindow(window);
}

bool r3_init(const char *title, v2i dim)
{
    if (!r3_sdl_init_video()) return false;
    r3_sdl_set_gl_attributes();
    if (!(window = r3_sdl_create_window(title, dim.x, dim.y))) return false;
    if (!r3_sdl_create_gl_context()) {
    SDL_DestroyWindow(window);
        return false;
    }
    window_dim = dim;
    r3_load_shaders();
    r3_make_cube();
    r3_make_quad();
    r3_enable_tests();
    return true;
}

#define SETUP_PROGRAM(base) \
	const char *vsh = (const char *)shader_##base##_vert; \
	const int vlen = shader_##base##_vert_len; \
	const char *fsh = (const char *)shader_##base##_frag; \
	const int  flen = shader_##base##_frag_len; \
    sh_##base.program = r3_make_program_from_src(vsh, vlen, fsh, flen); \
    glUseProgram(sh_##base.program);

#define ATTRIB_LOC(base, loc) \
    sh_##base.loc = glGetAttribLocation(sh_##base.program, #loc);

#define UNIFORM_LOC(base, loc) \
    sh_##base.loc = glGetUniformLocation(sh_##base.program, #loc);

#include "../shader/normal.vert.h"
#include "../shader/normal.frag.h"

struct {
  GLuint program;
  GLint a_position, a_normal;
  GLint u_mvp;
  GLint u_normal;
  GLint u_light_position;
  GLint u_ambient;
  GLint u_diffuse;
  GLint u_specular;
  GLint u_shininess;
  GLint u_sample;
} sh_normal;

void r3_make_normal_shader()
{
  SETUP_PROGRAM(normal)
  ATTRIB_LOC(normal, a_position)
  ATTRIB_LOC(normal, a_normal)
  UNIFORM_LOC(normal, u_mvp)
  UNIFORM_LOC(normal, u_normal)
  UNIFORM_LOC(normal, u_light_position)
  UNIFORM_LOC(normal, u_ambient)
  UNIFORM_LOC(normal, u_diffuse)
  UNIFORM_LOC(normal, u_specular)
  UNIFORM_LOC(normal, u_shininess)
  UNIFORM_LOC(normal, u_sample)
}

#include "../shader/cell.vert.h"
#include "../shader/cell.frag.h"

struct {
  GLuint program;
  GLint a_position;
  GLint a_normal;
  GLint u_mvp;
  GLint u_normal;
  GLint u_light_position;
  GLint u_ambient;
  GLint u_diffuse;
  GLint u_specular;
  GLint u_shininess;
  GLint u_sample;
} sh_cell;

void r3_make_cell_shader()
{
  SETUP_PROGRAM(cell)
  ATTRIB_LOC(cell, a_position)
  ATTRIB_LOC(cell, a_normal)
  UNIFORM_LOC(cell, u_mvp)
  UNIFORM_LOC(cell, u_normal)
  UNIFORM_LOC(cell, u_light_position)
  UNIFORM_LOC(cell, u_ambient)
  UNIFORM_LOC(cell, u_diffuse)
  UNIFORM_LOC(cell, u_specular)
  UNIFORM_LOC(cell, u_shininess)
  UNIFORM_LOC(cell, u_sample)
}

#include "../shader/color.vert.h"
#include "../shader/color.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_color;
	GLint u_mvp;
} sh_color;

void r3_make_color_shader()
{
  SETUP_PROGRAM(color)
  ATTRIB_LOC(color, a_position)
  ATTRIB_LOC(color, a_color)
  UNIFORM_LOC(color, u_mvp)
}

#include "../shader/texture.vert.h"
#include "../shader/texture.frag.h"

struct {
  GLuint program;
  GLint a_position;
  GLint a_texcoord;
  GLint u_mvp;
  GLint u_sample;
} sh_texture;

void r3_make_texture_shader()
{
  SETUP_PROGRAM(texture)
  ATTRIB_LOC(texture, a_position)
  ATTRIB_LOC(texture, a_texcoord)
  UNIFORM_LOC(texture, u_mvp)
  UNIFORM_LOC(texture, u_sample)
}

#include "../shader/color_normal_texture.vert.h"
#include "../shader/color_normal_texture.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_normal;
	GLint a_color;
	GLint a_texcoord;
	GLint u_mvp;
	GLint u_normal;
	GLint u_light_position;
	GLint u_ambient;
	GLint u_specular;
	GLint u_shininess;
	GLint u_sample;
} sh_color_normal_texture;

void r3_make_color_normal_texture_shader()
{
  SETUP_PROGRAM(color_normal_texture)
  ATTRIB_LOC(color_normal_texture, a_position)
  ATTRIB_LOC(color_normal_texture, a_normal)
  ATTRIB_LOC(color_normal_texture, a_color)
  ATTRIB_LOC(color_normal_texture, a_texcoord)
  UNIFORM_LOC(color_normal_texture, u_mvp)
  UNIFORM_LOC(color_normal_texture, u_normal)
  UNIFORM_LOC(color_normal_texture, u_light_position)
  UNIFORM_LOC(color_normal_texture, u_ambient)
  UNIFORM_LOC(color_normal_texture, u_specular)
  UNIFORM_LOC(color_normal_texture, u_shininess)
  UNIFORM_LOC(color_normal_texture, u_sample)
}

#include "../shader/blit.vert.h"
#include "../shader/blit.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_texcoord;
	GLint u_sample;
} sh_blit;

void r3_make_blit_shader()
{
  SETUP_PROGRAM(blit)
  ATTRIB_LOC(blit, a_position)
  ATTRIB_LOC(blit, a_texcoord)
  UNIFORM_LOC(blit, u_sample)
}

#include "../shader/blit_alpha.vert.h"
#include "../shader/blit_alpha.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_texcoord;
	GLint u_sample;
	GLint u_alpha;
} sh_blit_alpha;

void r3_make_blit_alpha_shader()
{
  SETUP_PROGRAM(blit_alpha)
  ATTRIB_LOC(blit_alpha, a_position)
  ATTRIB_LOC(blit_alpha, a_texcoord)
  UNIFORM_LOC(blit_alpha, u_sample)
  UNIFORM_LOC(blit_alpha, u_alpha)
}

#include "../shader/gaussian.vert.h"
#include "../shader/gaussian.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_texcoord;
	GLint u_sample;
	GLint u_coefficients;
	GLint u_offset;
} sh_gaussian;

void r3_make_gaussian_shader()
{
  SETUP_PROGRAM(gaussian)
  ATTRIB_LOC(gaussian, a_position)
  ATTRIB_LOC(gaussian, a_texcoord)
  UNIFORM_LOC(gaussian, u_sample)
  UNIFORM_LOC(gaussian, u_coefficients)
  UNIFORM_LOC(gaussian, u_offset)

  const float kernel[3] = { 5.0f / 16.0f, 6.0f / 16.0f, 5.0f / 16.0f };   
  glUniform1fv(sh_gaussian.u_coefficients, 3, kernel);
}

#include "../shader/high_pass.vert.h"
#include "../shader/high_pass.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_texcoord;
	GLint u_sample;
	GLint u_threshold;
} sh_high_pass;

void r3_make_high_pass_shader()
{
  SETUP_PROGRAM(high_pass)
  ATTRIB_LOC(high_pass, a_position)
  ATTRIB_LOC(high_pass, a_texcoord)
  UNIFORM_LOC(high_pass, u_sample)
  UNIFORM_LOC(high_pass, u_threshold)

  glUniform1f(sh_high_pass.u_threshold, 0.85);
}

#include "../shader/light.vert.h"
#include "../shader/light.frag.h"

struct {
	GLuint program;
	GLint a_position;
	GLint a_normal;
	GLint u_mvp;
	GLint u_normal;
	GLint u_light_position;
	GLint u_ambient;
	GLint u_diffuse;
	GLint u_specular;
	GLint u_shininess;
} sh_light;

void r3_make_light_shader()
{
  SETUP_PROGRAM(light)
  ATTRIB_LOC(light, a_position)
  ATTRIB_LOC(light, a_normal)
  UNIFORM_LOC(light, u_normal)
  UNIFORM_LOC(light, u_light_position)
  UNIFORM_LOC(light, u_ambient)
  UNIFORM_LOC(light, u_diffuse)
  UNIFORM_LOC(light, u_specular)
  UNIFORM_LOC(light, u_shininess)
}

void r3_load_shaders()
{

  r3_make_normal_shader();
  r3_make_cell_shader();
  r3_make_texture_shader();
  r3_make_color_shader();
  r3_make_color_normal_texture_shader();
  r3_make_blit_shader();
  r3_make_blit_alpha_shader();
  r3_make_gaussian_shader();
  r3_make_high_pass_shader();
  r3_make_light_shader();
}

void r3_clear(v3f color, unsigned int clear_bits)
{
	glClearColor(color.x, color.y, color.z, 1);
	glClear(clear_bits);
}

//void r3_render(struct r3_ren *ren)
//{
// ren->render(ren);
//}

void r3_viewport()
{
	glViewport(0, 0, window_dim.x, window_dim.y);
}

void  r3_enable_tests()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

//void r3_quit(struct r3_ren *ren)
//{
//	ren->quit(ren);
//}

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

GLuint r3_make_shader(const char *src, GLenum type, int src_len)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, src_len == 0 ? NULL : &src_len);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar *info = calloc(len, sizeof(GLchar));
		glGetShaderInfoLog(shader, len, NULL, info);
		fprintf(stderr, "glCompileShader failed:\n%s\n", info);
		free(info);
	}
	return shader;
}

GLuint r3_load_shader(const char *path, GLenum type)
{
	char *data = r3_load_file(path);
	assert(data != NULL);
	GLuint result = r3_make_shader(data, type, 0);
	free(data);
	return result;
} 

GLuint r3_make_program(GLuint vert_shader, GLuint frag_shader)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == false) {
		int len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		GLchar *info = calloc(len, sizeof(GLchar));
		glGetProgramInfoLog(program, len, NULL, info);
		fprintf(stderr, "glLinkProgram failed: %s\n", info);
		free(info);
	}
	glDetachShader(program, vert_shader);
	glDetachShader(program, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	return program;
}

GLuint r3_make_program_from_src(const char *vsh, int vsh_len, const char *fsh, int fsh_len)
{
	const GLuint vert = r3_make_shader(vsh, GL_VERTEX_SHADER, vsh_len);
	const GLuint frag = r3_make_shader(fsh, GL_FRAGMENT_SHADER, fsh_len);
	return r3_make_program(vert, frag);
}

unsigned int r3_make_program_from_src_unsigned(const unsigned char *vsh, int vsh_len, const unsigned char *fsh, int fsh_len)
{
	return r3_make_program_from_src((const char *)vsh, vsh_len, (const char*)fsh, fsh_len);
}

unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path)
{
	unsigned int vert = r3_load_shader(vert_path, GL_VERTEX_SHADER);
	unsigned int frag = r3_load_shader(frag_path, GL_FRAGMENT_SHADER);
	return r3_make_program(vert, frag);
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

void r3_make_cube()
{
    // TODO: clean up later
    struct r3_spec *spec = r3_create_cuboid_spec();
    r3_make_mesh_from_spec(spec, &mesh_cube);
    free(spec);
}

void r3_make_quad()
{
	struct r3_pt verts[4] = {
		(struct r3_pt) {
			.position = _v3f(-1, 1, 1),
			.texcoord = _v2f(0, 1)
		},
		(struct r3_pt) {
			.position = _v3f(-1,-1, 1),
			.texcoord = _v2f(0, 0)
		},
		(struct r3_pt) {
			.position = _v3f( 1, 1, 1),
			.texcoord = _v2f(1, 1)
		},
		(struct r3_pt) {
			.position = _v3f( 1,-1, 1),
			.texcoord = _v2f(1, 0)
		},
	};
	unsigned short int indices[6] = {
		0, 1, 2,
		1, 3, 2,
	};
    struct r3_spec spec;
    spec.verts.tag = R3_VERTS_PT;
    spec.verts.len = 4;
    spec.verts.pt = verts;
    spec.indices.tag = R3_INDICES_USHORT;
    spec.indices.len = 6;
    spec.indices.data = indices;
    r3_make_mesh_from_spec(&spec, &mesh_quad);  

    /* 
	mesh_quad.num_indices = 6;
	glGenBuffers(1, &mesh_quad.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_quad.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(struct r3_pt), verts, GL_STATIC_DRAW);
	glGenBuffers(1, &mesh_quad.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_quad.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_quad.num_indices * sizeof(unsigned short int), indices, GL_STATIC_DRAW);
	mesh_quad.verts_tag = R3_VERTS_PT;
    */
}


unsigned int r3_make_fbo_tex(int width, int height)
{
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	return tex;
}


/*
static void render_drawable(const struct r3_mesh *m, const struct r3_shader *sh, int flags)
{
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position);
	glVertexAttribPointer(sh->attrib.position, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));

	if (flags == R3_TEXCOORD) {
		glEnableVertexAttribArray(sh->attrib.texcoord);
		glVertexAttribPointer(sh->attrib.texcoord, 2, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
    }
    if (flags == R3_NORMAL) {
		glEnableVertexAttribArray(sh->attrib.normal);
		glVertexAttribPointer(sh->attrib.normal, 3, GL_FLOAT, GL_FALSE,
			r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(sh->attrib.position);
	if (flags == R3_TEXCOORD)
		glDisableVertexAttribArray(sh->attrib.texcoord);
	if (flags == R3_NORMAL)
		glDisableVertexAttribArray(sh->attrib.normal);
}
*/

void r3_blit_alpha_render(const struct r3_mesh *m, unsigned int tex, float alpha)
{
    glUseProgram(sh_blit_alpha.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_blit_alpha.u_sample, 0);
    glUniform1f(sh_blit_alpha.u_alpha, alpha);

    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_blit_alpha.a_position);

	glVertexAttribPointer(sh_blit_alpha.a_position, 3, GL_FLOAT, GL_FALSE,
    r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));

    glEnableVertexAttribArray(sh_blit_alpha.a_texcoord);
	glVertexAttribPointer(sh_blit_alpha.a_texcoord, 2, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag),
        (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
}

void r3_texcoord_render(const struct r3_mesh *m, GLint a_position, GLint a_texcoord)
{
    // position	
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_blit.a_position);
    glVertexAttribPointer(sh_blit.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));

    // texcoord
    glEnableVertexAttribArray(sh_blit.a_texcoord);
    glVertexAttribPointer(sh_blit.a_texcoord, 2, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));

    // position
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, 0);
    glDisableVertexAttribArray(sh_blit.a_position);

    // texcoord
    glDisableVertexAttribArray(sh_blit.a_texcoord);
}

void r3_render_blit_alpha(const struct r3_mesh *m, unsigned int tex, float alpha)
{
	glUseProgram(sh_blit_alpha.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(sh_blit_alpha.u_sample, 0);
	glUniform1f(sh_blit_alpha.u_alpha, alpha);

    r3_texcoord_render(m, sh_blit_alpha.a_position, sh_blit_alpha.a_texcoord);
}

void r3_render_blit(const struct r3_mesh *m, unsigned int tex)
{
	glUseProgram(sh_blit.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(sh_blit.u_sample, 0);

    r3_texcoord_render(m, sh_blit.a_position, sh_blit.a_texcoord);
}

void r3_render_blur_width(const struct r3_mesh *m, unsigned int tex, float aspect, float width)
{
	glUseProgram(sh_gaussian.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(sh_gaussian.u_sample, 0);
	glUniform2f(sh_gaussian.u_offset, aspect / width, 0);
    
    r3_texcoord_render(m, sh_gaussian.a_position, sh_gaussian.a_texcoord);
}

void r3_render_blur_height(const struct r3_mesh *m, unsigned int tex, float aspect, float height)
{
	glUseProgram(sh_gaussian.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(sh_gaussian.u_sample, 0);
	glUniform2f(sh_gaussian.u_offset, 0, aspect / height);
    
    r3_texcoord_render(m, sh_gaussian.a_position, sh_gaussian.a_texcoord);
}

void r3_render_high_pass(const struct r3_mesh *m, unsigned int tex)
{
	glUseProgram(sh_high_pass.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(sh_high_pass.u_sample, 0);
    
    r3_texcoord_render(m, sh_high_pass.a_position, sh_high_pass.a_texcoord);
}

/*
void render_light(const struct r3_mesh *m, const struct r3_shader *sh, m4f mv, m4f mvp)
{
	glUseProgram(sh->program);
	glUniformMatrix4fv(sh->uniform.mvp, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh->uniform.normal, 1, GL_FALSE, m3m4f(mv).val);
	glUniform3fv(sh->uniform.light_position, 1, light_pos.val);
	glUniform3fv(sh->uniform.ambient, 1, ambient.val);
	glUniform3fv(sh->uniform.diffuse, 1, diffuse.val);
	glUniform3fv(sh->uniform.specular, 1, specular.val);
	glUniform1f(sh->uniform.shininess, shininess);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glEnableVertexAttribArray(sh->attrib.position);
	glEnableVertexAttribArray(sh->attrib.normal);
	glVertexAttribPointer(sh->attrib.position, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glVertexAttribPointer(sh->attrib.normal, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(sh->attrib.position);
	glDisableVertexAttribArray(sh->attrib.normal);
}
*/

void r3_render_normal(const struct r3_mesh *m, m4f mv, m4f mvp,
		v3f light_position, v3f ambient, v3f diffuse, v3f specular, float shininess)
{
	glUseProgram(sh_normal.program);
	// Set uniforms
	glUniformMatrix4fv(sh_normal.u_mvp, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh_normal.u_normal, 1, 0, m3m4f(mv).val);
	glUniform3fv(sh_normal.u_light_position, 1, light_position.val);
	glUniform3fv(sh_normal.u_ambient, 1, ambient.val);
	glUniform3fv(sh_normal.u_diffuse, 1, diffuse.val);
	glUniform3fv(sh_normal.u_specular, 1, specular.val);
	glUniform1f(sh_normal.u_shininess, shininess);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh_normal.a_position);
	glVertexAttribPointer(sh_normal.a_position, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glEnableVertexAttribArray(sh_normal.a_normal);
	glVertexAttribPointer(sh_normal.a_normal, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh_normal.a_position);
	glDisableVertexAttribArray(sh_normal.a_normal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_texture(const struct r3_mesh *m, unsigned int tex, m4f mvp)
{
	const enum r3_verts_tag vt = m->verts_tag;
	glUseProgram(sh_texture.program);
	// Set uniforms
	glUniformMatrix4fv(sh_texture.u_mvp, 1, GL_FALSE, mvp.val);
	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(sh_texture.u_sample, 0);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh_texture.a_position);
	glVertexAttribPointer(sh_texture.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
	glEnableVertexAttribArray(sh_texture.a_texcoord);
	glVertexAttribPointer(sh_texture.a_texcoord, 2, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_TEXCOORD));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh_texture.a_position);
	glDisableVertexAttribArray(sh_texture.a_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_color(const struct r3_mesh *m, m4f mvp)
{
	const enum r3_verts_tag vt = m->verts_tag;
	glUseProgram(sh_color.program);
	// Set uniforms
	glUniformMatrix4fv(sh_color.u_mvp, 1, GL_FALSE, mvp.val);
	// Set texture
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh_color.a_position);
	glVertexAttribPointer(sh_color.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
	glEnableVertexAttribArray(sh_color.a_color);
	glVertexAttribPointer(sh_color.a_color, 3, GL_FLOAT, GL_FALSE,
		r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_COLOR));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh_color.a_position);
	glDisableVertexAttribArray(sh_color.a_color);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_color_normal_texture(const struct r3_mesh *m, unsigned int tex,
		m4f mv, m4f mvp, v3f light_position, v3f ambient, v3f specular, float shininess)
{
	const enum r3_verts_tag vt = m->verts_tag;
	glUseProgram(sh_color_normal_texture.program);
	// Set uniforms
	glUniformMatrix4fv(sh_color_normal_texture.u_mvp, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(sh_color_normal_texture.u_normal, 1, 0, m3m4f(mv).val);
	glUniform3fv(sh_color_normal_texture.u_light_position, 1, light_position.val);
	glUniform3fv(sh_color_normal_texture.u_ambient, 1, ambient.val);
	glUniform3fv(sh_color_normal_texture.u_specular, 1, specular.val);
	glUniform1f(sh_color_normal_texture.u_shininess, shininess);
	// Set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(sh_color_normal_texture.u_sample, 0);
	// VBO & IBO 
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh_color_normal_texture.a_position);
	glVertexAttribPointer(sh_color_normal_texture.a_position, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
	glEnableVertexAttribArray(sh_color_normal_texture.a_color);
	glVertexAttribPointer(sh_color_normal_texture.a_color, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_COLOR));
	glEnableVertexAttribArray(sh_color_normal_texture.a_normal);
	glVertexAttribPointer(sh_color_normal_texture.a_normal, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_NORMAL));
	glEnableVertexAttribArray(sh_color_normal_texture.a_texcoord);
	glVertexAttribPointer(sh_color_normal_texture.a_texcoord, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(sh_color_normal_texture.a_position);
	glDisableVertexAttribArray(sh_color_normal_texture.a_color);
	glDisableVertexAttribArray(sh_color_normal_texture.a_normal);
	glDisableVertexAttribArray(sh_color_normal_texture.a_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int r3_load_tga_texture(const char *path)
{
	int width, height;
	char *data = r3_load_tga(path, &width, &height);
	if (data) {
		unsigned int tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		free(data);
		return tex;
	}
	return 0;
}

void r3_break_mesh(const struct r3_mesh *m)
{
	glDeleteBuffers(1, &m->vbo);
	glDeleteBuffers(1, &m->ibo);
}

void *r3_offset_ptr(enum r3_verts_tag tag, enum r3_vert vert)
{
	return (void*)r3_offset(tag, vert);
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

const r3_mesh_t *r3_cube()
{
    return &mesh_cube;
}

const r3_mesh_t *r3_quad()
{
    return &mesh_quad;
}

void r3_quit()
{
    r3_break_mesh(&mesh_cube);
    r3_break_mesh(&mesh_quad);
    glDeleteProgram(sh_normal.program);
    glDeleteProgram(sh_cell.program);
    glDeleteProgram(sh_color.program);
    glDeleteProgram(sh_texture.program);
    glDeleteProgram(sh_color_normal_texture.program);
    glDeleteProgram(sh_blit.program);
    glDeleteProgram(sh_blit_alpha.program);
    glDeleteProgram(sh_gaussian.program);
    glDeleteProgram(sh_high_pass.program);
    glDeleteProgram(sh_light.program);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_DestroyWindow(window);
}
