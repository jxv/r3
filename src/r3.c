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
struct r3_mesh cube_mesh, quad_mesh;

static bool r3_sdl_init_video()
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Couldn't initialize ren driver: %s\n", SDL_GetError());
        return false;
    }
    return true;
} 

static void r3_sdl_set_gl_attributes() {
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

static SDL_Window *r3_sdl_create_window(const char *title, int width, int height) {
    return SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height,
        SDL_WINDOW_OPENGL
    );
}

static bool r3_sdl_create_gl_context() {
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

void r3_render() {
    SDL_GL_SwapWindow(window);
}

bool r3_init(const char *title, int w, int h) {
    if (!r3_sdl_init_video()) {
        return false;
    }
    r3_sdl_set_gl_attributes();
    if (!(window = r3_sdl_create_window(title, w, h))) {
        return false;
    }
    if (!r3_sdl_create_gl_context()) {
        SDL_DestroyWindow(window);
        return false;
    }
    window_dim.x = w;
    window_dim.y = h;
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

static void make_sh_cell() {
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

#include "../shader/c.vert.h"
#include "../shader/c.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_color;
    GLint u_mvp;
} sh_c;

static void make_sh_c() {
    SETUP_PROGRAM(c)
    ATTRIB_LOC(c, a_position)
    ATTRIB_LOC(c, a_color)
    UNIFORM_LOC(c, u_mvp)
}

#include "../shader/k.vert.h"
#include "../shader/k.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint u_mvp;
    GLint u_kolor;
} sh_k;

static void make_sh_k() {
    SETUP_PROGRAM(k)
    ATTRIB_LOC(k, a_position)
    UNIFORM_LOC(k, u_mvp)
    UNIFORM_LOC(k, u_kolor)
}

#include "../shader/n.vert.h"
#include "../shader/n.frag.h"

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
} sh_n;

static void make_sh_n() {
    SETUP_PROGRAM(n)
    ATTRIB_LOC(n, a_position)
    ATTRIB_LOC(n, a_normal)
    UNIFORM_LOC(n, u_mvp)
    UNIFORM_LOC(n, u_normal)
    UNIFORM_LOC(n, u_light_position)
    UNIFORM_LOC(n, u_ambient)
    UNIFORM_LOC(n, u_diffuse)
    UNIFORM_LOC(n, u_specular)
    UNIFORM_LOC(n, u_shininess)
    UNIFORM_LOC(n, u_sample)
}

#include "../shader/t.vert.h"
#include "../shader/t.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_texcoord;
    GLint u_mvp;
    GLint u_sample;
} sh_t;

static void make_sh_t() {
    SETUP_PROGRAM(t)
    ATTRIB_LOC(t, a_position)
    ATTRIB_LOC(t, a_texcoord)
    UNIFORM_LOC(t, u_mvp)
    UNIFORM_LOC(t, u_sample)
}

#include "../shader/cn.vert.h"
#include "../shader/cn.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_color;
    GLint a_normal;
    GLint u_mvp;
    GLint u_normal;
    GLint u_light_position;
    GLint u_ambient;
    GLint u_specular;
    GLint u_shininess;
} sh_cn;

static void make_sh_cn() {
    SETUP_PROGRAM(cn)
    ATTRIB_LOC(cn, a_position)
    ATTRIB_LOC(cn, a_color)
    ATTRIB_LOC(cn, a_normal)
    UNIFORM_LOC(cn, u_mvp)
    UNIFORM_LOC(cn, u_normal)
    UNIFORM_LOC(cn, u_light_position)
    UNIFORM_LOC(cn, u_ambient)
    UNIFORM_LOC(cn, u_specular)
    UNIFORM_LOC(cn, u_shininess)
}

#include "../shader/kn.vert.h"
#include "../shader/kn.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_normal;
    GLint u_mvp;
    GLint u_kolor;
    GLint u_normal;
    GLint u_light_position;
    GLint u_ambient;
    GLint u_specular;
    GLint u_shininess;
} sh_kn;

static void make_sh_kn() {
    SETUP_PROGRAM(kn)
    ATTRIB_LOC(kn, a_position)
    ATTRIB_LOC(kn, a_normal)
    UNIFORM_LOC(kn, u_mvp)
    UNIFORM_LOC(kn, u_kolor)
    UNIFORM_LOC(kn, u_normal)
    UNIFORM_LOC(kn, u_light_position)
    UNIFORM_LOC(kn, u_ambient)
    UNIFORM_LOC(kn, u_specular)
    UNIFORM_LOC(kn, u_shininess)
}

#include "../shader/ct.vert.h"
#include "../shader/ct.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_color;
    GLint a_texcoord;
    GLint u_mvp;
    GLint u_sample;
} sh_ct;

static void make_sh_ct() {
    SETUP_PROGRAM(ct)
    ATTRIB_LOC(ct, a_position)
    ATTRIB_LOC(ct, a_color)
    ATTRIB_LOC(ct, a_texcoord)
    UNIFORM_LOC(ct, u_mvp)
    UNIFORM_LOC(ct, u_sample)
}

#include "../shader/kt.vert.h"
#include "../shader/kt.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_texcoord;
    GLint u_mvp;
    GLint u_kolor;
    GLint u_sample;
} sh_kt;

static void make_sh_kt() {
    SETUP_PROGRAM(kt)
    ATTRIB_LOC(kt, a_position)
    ATTRIB_LOC(kt, a_texcoord)
    UNIFORM_LOC(kt, u_mvp)
    UNIFORM_LOC(kt, u_kolor)
    UNIFORM_LOC(kt, u_sample)
}

#include "../shader/nt.vert.h"
#include "../shader/nt.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_normal;
    GLint a_texcoord;
    GLint u_mvp;
    GLint u_normal;
    GLint u_light_position;
    GLint u_ambient;
    GLint u_specular;
    GLint u_shininess;
    GLint u_sample;
} sh_nt;

static void make_sh_nt() {
    SETUP_PROGRAM(nt)
    ATTRIB_LOC(nt, a_position)
    ATTRIB_LOC(nt, a_normal)
    ATTRIB_LOC(nt, a_texcoord)
    UNIFORM_LOC(nt, u_mvp)
    UNIFORM_LOC(nt, u_normal)
    UNIFORM_LOC(nt, u_light_position)
    UNIFORM_LOC(nt, u_ambient)
    UNIFORM_LOC(nt, u_specular)
    UNIFORM_LOC(nt, u_shininess)
    UNIFORM_LOC(nt, u_sample)
}

#include "../shader/cnt.vert.h"
#include "../shader/cnt.frag.h"

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
} sh_cnt;

static void make_sh_cnt() {
    SETUP_PROGRAM(cnt)
    ATTRIB_LOC(cnt, a_position)
    ATTRIB_LOC(cnt, a_normal)
    ATTRIB_LOC(cnt, a_color)
    ATTRIB_LOC(cnt, a_texcoord)
    UNIFORM_LOC(cnt, u_mvp)
    UNIFORM_LOC(cnt, u_normal)
    UNIFORM_LOC(cnt, u_light_position)
    UNIFORM_LOC(cnt, u_ambient)
    UNIFORM_LOC(cnt, u_specular)
    UNIFORM_LOC(cnt, u_shininess)
    UNIFORM_LOC(cnt, u_sample)
}

#include "../shader/knt.vert.h"
#include "../shader/knt.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_normal;
    GLint a_texcoord;
    GLint u_mvp;
    GLint u_kolor;
    GLint u_normal;
    GLint u_light_position;
    GLint u_ambient;
    GLint u_specular;
    GLint u_shininess;
    GLint u_sample;
} sh_knt;

static void make_sh_knt() {
    SETUP_PROGRAM(knt)
    ATTRIB_LOC(knt, a_position)
    ATTRIB_LOC(knt, a_normal)
    ATTRIB_LOC(knt, a_texcoord)
    UNIFORM_LOC(knt, u_mvp)
    UNIFORM_LOC(knt, u_kolor)
    UNIFORM_LOC(knt, u_normal)
    UNIFORM_LOC(knt, u_light_position)
    UNIFORM_LOC(knt, u_ambient)
    UNIFORM_LOC(knt, u_specular)
    UNIFORM_LOC(knt, u_shininess)
    UNIFORM_LOC(knt, u_sample)
}

#include "../shader/blit.vert.h"
#include "../shader/blit.frag.h"

struct {
    GLuint program;
    GLint a_position;
    GLint a_texcoord;
    GLint u_sample;
} sh_blit;

static void make_sh_blit() {
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

static void make_sh_blit_alpha() {
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

static void make_sh_gaussian() {
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

static void make_sh_high_pass() {
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

static void make_sh_light() {
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

void r3_load_shaders() {
    make_sh_c();
    make_sh_k();
    make_sh_n();
    make_sh_t();
    make_sh_cn();
    make_sh_kn();
    make_sh_ct();
    make_sh_kt();
    make_sh_nt();
    make_sh_cnt();
    make_sh_knt();
    make_sh_cell();
    make_sh_blit();
    make_sh_blit_alpha();
    make_sh_gaussian();
    make_sh_high_pass();
    make_sh_light();
}

void r3_clear(const v3f *color, r3_clear_bit_t bits) {
    glClearColor(color->x, color->y, color->z, 1);
    glClear(bits);
}

void r3_viewport() {
    glViewport(0, 0, window_dim.x, window_dim.y);
}

void  r3_enable_tests() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
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

char* r3_load_tga_bgr(const char *fileName, int *width, int *height) {
    char *buf = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;
    f = fopen(fileName, "rb");
    if (f == NULL) {
        return NULL;
    }
    if (tgaheader[2] == 2) {
        return NULL; // compression not supported
    }
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

char* r3_load_tga_rgb(const char *fileName, int *width, int *height) {
    char *buf = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;
    f = fopen(fileName, "rb");
    if (f == NULL) {
        return NULL;
    }
    if (tgaheader[2] == 2) {
        return NULL; // compression not supported
    }
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
    for (unsigned int i = 0; i < imagesize; i += 3) {
        const char b = buf[i];
        const char r = buf[i+2];
        buf[i] = r;
        buf[i+2] = b;
    }
    return buf;
}

GLuint r3_make_shader(const char *src, GLenum type, int src_len) {
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

GLuint r3_load_shader(const char *path, GLenum type) {
    char *data = r3_load_file(path);
    assert(data != NULL);
    GLuint result = r3_make_shader(data, type, 0);
    free(data);
    return result;
} 

GLuint r3_make_program(GLuint vert_shader, GLuint frag_shader) {
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

GLuint r3_make_program_from_src(const char *vsh, int vsh_len, const char *fsh, int fsh_len) {
    const GLuint vert = r3_make_shader(vsh, GL_VERTEX_SHADER, vsh_len);
    const GLuint frag = r3_make_shader(fsh, GL_FRAGMENT_SHADER, fsh_len);
    return r3_make_program(vert, frag);
}

unsigned int r3_make_program_from_src_unsigned(const unsigned char *vsh, int vsh_len, const unsigned char *fsh, int fsh_len) {
    return r3_make_program_from_src((const char *)vsh, vsh_len, (const char*)fsh, fsh_len);
}

unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path) {
    unsigned int vert = r3_load_shader(vert_path, GL_VERTEX_SHADER);
    unsigned int frag = r3_load_shader(frag_path, GL_FRAGMENT_SHADER);
    return r3_make_program(vert, frag);
}

ssize_t r3_verts_tag_sizeof(r3_verts_tag_t tag) {
    switch (tag) {
    case R3_VERTS_C: return sizeof(r3_c_t);
    case R3_VERTS_N: return sizeof(r3_n_t);
    case R3_VERTS_CN: return sizeof(r3_cn_t);
    case R3_VERTS_T: return sizeof(r3_t_t);
    case R3_VERTS_NT: return sizeof(r3_nt_t);
    case R3_VERTS_CNT: return sizeof(r3_cnt_t);
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

void r3_make_cube() {
    // TODO: clean up later
    struct r3_spec *spec = r3_create_cuboid_spec();
    r3_make_mesh_from_spec(spec, &cube_mesh);
    free(spec);
}

void r3_make_quad() {
    r3_t_t verts[4] = {
        (r3_t_t) {
            .position = _v3f(-1, 1, 1),
            .texcoord = _v2f(0, 1)
        },
        (struct r3_t) {
            .position = _v3f(-1,-1, 1),
            .texcoord = _v2f(0, 0)
        },
        (struct r3_t) {
            .position = _v3f( 1, 1, 1),
            .texcoord = _v2f(1, 1)
        },
        (struct r3_t) {
            .position = _v3f( 1,-1, 1),
            .texcoord = _v2f(1, 0)
        },
    };
    unsigned short int indices[6] = {
        0, 1, 2,
        1, 3, 2,
    };
    struct r3_spec spec;
    spec.verts.tag = R3_VERTS_T;
    spec.verts.len = 4;
    spec.verts.t = verts;
    spec.indices.tag = R3_INDICES_USHORT;
    spec.indices.len = 6;
    spec.indices.data = indices;
    r3_make_mesh_from_spec(&spec, &quad_mesh);  

    /* 
    quad_mesh.num_indices = 6;
    glGenBuffers(1, &quad_mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(struct r3_pt), verts, GL_STATIC_DRAW);
    glGenBuffers(1, &quad_mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.num_indices * sizeof(unsigned short int), indices, GL_STATIC_DRAW);
    quad_mesh.verts_tag = R3_VERTS_PT;
    */
}


unsigned int r3_make_fbo_tex(int width, int height) {
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
// TODO: Delete this after all effects are reimplemented
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

void r3_render_range_c(const r3_mesh_t *m, 
        int start_idx, int end_idx,
        const m4f *mvp) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_c.program);
    // Set uniforms
    glUniformMatrix4fv(sh_c.u_mvp, 1, GL_FALSE, mvp->val);
    // Set texture
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_c.a_position);
    glVertexAttribPointer(sh_c.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_c.a_color);
    glVertexAttribPointer(sh_c.a_color, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_COLOR));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_c.a_position);
    glDisableVertexAttribArray(sh_c.a_color);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_c(const r3_mesh_t *m,
        const m4f *mvp) {
    r3_render_range_c(m, 0, m->num_indices - 1, mvp);
}

void r3_render_range_k(const r3_mesh_t *m, 
        int start_idx, int end_idx,
        const m4f *mvp, const v3f *kolor) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_k.program);
    // Set uniforms
    glUniformMatrix4fv(sh_k.u_mvp, 1, GL_FALSE, mvp->val);
    glUniform3fv(sh_k.u_kolor, 1, kolor->val);
    // Set texture
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_k.a_position);
    glVertexAttribPointer(sh_k.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_k.a_position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_k(const r3_mesh_t *m,
        const m4f *mvp, const v3f *kolor) {
        r3_render_range_k(m, 0, m->num_indices - 1, mvp, kolor);
}

void r3_render_range_n(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const r3_normal_t *n) {
    glUseProgram(sh_n.program);
    // Set uniforms
    glUniformMatrix4fv(sh_n.u_mvp, 1, GL_FALSE, mvp->val);
    glUniformMatrix3fv(sh_n.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_n.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_n.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_n.u_diffuse, 1, n->diffuse_color.val);
    glUniform3fv(sh_n.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_n.u_shininess, n->shininess);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_n.a_position);
    glVertexAttribPointer(sh_n.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
    glEnableVertexAttribArray(sh_n.a_normal);
    glVertexAttribPointer(sh_n.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_n.a_position);
    glDisableVertexAttribArray(sh_n.a_normal);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_n(const r3_mesh_t *m,
        const m4f *mvp, const r3_normal_t *n) {
    r3_render_range_n(m, 0, m->num_indices - 1, mvp, n);
}

void r3_render_range_t(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_t.program);
    // Set uniforms
    glUniformMatrix4fv(sh_t.u_mvp, 1, GL_FALSE, mvp->val);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_t.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_t.a_position);
    glVertexAttribPointer(sh_t.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_t.a_texcoord);
    glVertexAttribPointer(sh_t.a_texcoord, 2, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_t.a_position);
    glDisableVertexAttribArray(sh_t.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_t(const r3_mesh_t *m,
        const m4f *mvp, unsigned int tex) {
    r3_render_range_t(m, 0, m->num_indices - 1, mvp, tex);
}

void r3_render_range_cn(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const r3_normal_t *n) {
    glUseProgram(sh_cn.program);
    // Set uniforms
    glUniformMatrix4fv(sh_cn.u_mvp, 1, GL_FALSE, mvp->val);
    glUniformMatrix3fv(sh_cn.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_cn.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_cn.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_cn.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_cn.u_shininess, n->shininess);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_cn.a_position);
    glVertexAttribPointer(sh_cn.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
     glEnableVertexAttribArray(sh_c.a_color);
    glVertexAttribPointer(sh_c.a_color, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_COLOR));
    glEnableVertexAttribArray(sh_cn.a_normal);
    glVertexAttribPointer(sh_cn.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
       glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_cn.a_position);
    glDisableVertexAttribArray(sh_cn.a_normal);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_cn(const r3_mesh_t *m,
        const m4f *mvp, const r3_normal_t *n) {
    r3_render_range_cn(m, 0, m->num_indices - 1, mvp, n);
}

void r3_render_range_kn(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const v3f *kolor, const r3_normal_t *n) {
    glUseProgram(sh_kn.program);
    // Set uniforms
    glUniformMatrix4fv(sh_kn.u_mvp, 1, GL_FALSE, mvp->val);
    glUniform3fv(sh_kn.u_kolor, 1, kolor->val);
    glUniformMatrix3fv(sh_kn.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_kn.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_kn.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_kn.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_kn.u_shininess, n->shininess);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_kn.a_position);
    glVertexAttribPointer(sh_kn.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
    glEnableVertexAttribArray(sh_kn.a_normal);
    glVertexAttribPointer(sh_kn.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
       glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_kn.a_position);
    glDisableVertexAttribArray(sh_kn.a_normal);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_kn(const r3_mesh_t *m,
        const m4f *mvp, const v3f *kolor, const r3_normal_t *n) {
    r3_render_range_kn(m, 0, m->num_indices - 1, mvp, kolor, n);
}


void r3_render_range_ct(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_ct.program);
    // Set uniforms
    glUniformMatrix4fv(sh_ct.u_mvp, 1, GL_FALSE, mvp->val);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_ct.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_ct.a_position);
    glVertexAttribPointer(sh_ct.a_position, 3, GL_FLOAT,
        GL_FALSE, r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_ct.a_color);
    glVertexAttribPointer(sh_ct.a_color, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_COLOR));
    glEnableVertexAttribArray(sh_ct.a_texcoord);
    glVertexAttribPointer(sh_ct.a_texcoord, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
       glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_ct.a_position);
    glDisableVertexAttribArray(sh_ct.a_color);
    glDisableVertexAttribArray(sh_ct.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_ct(const r3_mesh_t *m,
        const m4f *mvp, unsigned int tex) {
    r3_render_range_ct(m, 0, m->num_indices - 1, mvp, tex);
}

void r3_render_range_kt(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const v3f *kolor, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_kt.program);
    // Set uniforms
    glUniformMatrix4fv(sh_kt.u_mvp, 1, GL_FALSE, mvp->val);
    glUniform3fv(sh_kt.u_kolor, 1, kolor->val);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_kt.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_kt.a_position);
    glVertexAttribPointer(sh_kt.a_position, 3, GL_FLOAT,
        GL_FALSE, r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_kt.a_texcoord);
    glVertexAttribPointer(sh_kt.a_texcoord, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
       glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_kt.a_position);
    glDisableVertexAttribArray(sh_kt.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_kt(const r3_mesh_t *m,
        const m4f *mvp, const v3f *kolor, unsigned int tex) {
    r3_render_range_kt(m, 0, m->num_indices - 1, mvp, kolor, tex);
}

void r3_render_range_nt(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const r3_normal_t *n, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_cnt.program);
    // Set uniforms
    glUniformMatrix4fv(sh_cnt.u_mvp, 1, GL_FALSE, mvp->val);
    glUniformMatrix3fv(sh_cnt.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_cnt.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_cnt.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_cnt.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_cnt.u_shininess, n->shininess);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_cnt.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_cnt.a_position);
    glVertexAttribPointer(sh_cnt.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_cnt.a_normal);
    glVertexAttribPointer(sh_cnt.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_NORMAL));
    glEnableVertexAttribArray(sh_cnt.a_texcoord);
    glVertexAttribPointer(sh_cnt.a_texcoord, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_cnt.a_position);
    glDisableVertexAttribArray(sh_cnt.a_normal);
    glDisableVertexAttribArray(sh_cnt.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_nt(const r3_mesh_t *m,
        const m4f *mvp, const r3_normal_t *n, unsigned int tex) {
    r3_render_range_nt(m, 0, m->num_indices - 1, mvp, n, tex);
}

void r3_render_range_cnt(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const r3_normal_t *n, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_cnt.program);
    // Set uniforms
    glUniformMatrix4fv(sh_cnt.u_mvp, 1, GL_FALSE, mvp->val);
    glUniformMatrix3fv(sh_cnt.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_cnt.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_cnt.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_cnt.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_cnt.u_shininess, n->shininess);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_cnt.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_cnt.a_position);
    glVertexAttribPointer(sh_cnt.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_cnt.a_color);
    glVertexAttribPointer(sh_cnt.a_color, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_COLOR));
    glEnableVertexAttribArray(sh_cnt.a_normal);
    glVertexAttribPointer(sh_cnt.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_NORMAL));
    glEnableVertexAttribArray(sh_cnt.a_texcoord);
    glVertexAttribPointer(sh_cnt.a_texcoord, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_cnt.a_position);
    glDisableVertexAttribArray(sh_cnt.a_color);
    glDisableVertexAttribArray(sh_cnt.a_normal);
    glDisableVertexAttribArray(sh_cnt.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_cnt(const r3_mesh_t *m,
        const m4f *mvp, const r3_normal_t *n, unsigned int tex) {
        r3_render_range_cnt(m, 0, m->num_indices - 1, mvp, n, tex);
}

void r3_render_range_knt(const r3_mesh_t *m,
        int start_idx, int end_idx,
        const m4f *mvp, const v3f *kolor, const r3_normal_t *n, unsigned int tex) {
    const enum r3_verts_tag vt = m->verts_tag;
    glUseProgram(sh_knt.program);
    // Set uniforms
    glUniformMatrix4fv(sh_knt.u_mvp, 1, GL_FALSE, mvp->val);
    glUniform3fv(sh_knt.u_kolor, 1, kolor->val);
    glUniformMatrix3fv(sh_knt.u_normal, 1, 0, n->mv.val);
    glUniform3fv(sh_knt.u_light_position, 1, n->light_position.val);
    glUniform3fv(sh_knt.u_ambient, 1, n->ambient_color.val);
    glUniform3fv(sh_knt.u_specular, 1, n->specular_color.val);
    glUniform1f(sh_knt.u_shininess, n->shininess);
    // Set texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_knt.u_sample, 0);
    // VBO & IBO 
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glEnableVertexAttribArray(sh_knt.a_position);
    glVertexAttribPointer(sh_knt.a_position, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_POSITION));
    glEnableVertexAttribArray(sh_knt.a_normal);
    glVertexAttribPointer(sh_knt.a_normal, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), r3_offset_ptr(vt, R3_NORMAL));
    glEnableVertexAttribArray(sh_knt.a_texcoord);
    glVertexAttribPointer(sh_knt.a_texcoord, 3, GL_FLOAT, GL_FALSE,
        r3_verts_tag_sizeof(vt), (void*)r3_offset(vt, R3_TEXCOORD));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
    glDrawElements(GL_TRIANGLES, 1 + end_idx - start_idx, GL_UNSIGNED_SHORT,
        (void*)(sizeof(GLushort) * start_idx));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(sh_knt.a_position);
    glDisableVertexAttribArray(sh_knt.a_normal);
    glDisableVertexAttribArray(sh_knt.a_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void r3_render_knt(const r3_mesh_t *m,
        const m4f *mvp, const v3f *kolor, const r3_normal_t *n, unsigned int tex) {
    r3_render_range_knt(m, 0, m->num_indices - 1, mvp, kolor, n, tex);
}

void r3_blit_alpha_render(const struct r3_mesh *m, unsigned int tex, float alpha) {
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

void r3_texcoord_render(const struct r3_mesh *m, GLint a_position, GLint a_texcoord) {
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

void r3_render_blit_alpha(const struct r3_mesh *m, unsigned int tex, float alpha) {
    glUseProgram(sh_blit_alpha.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(sh_blit_alpha.u_sample, 0);
    glUniform1f(sh_blit_alpha.u_alpha, alpha);

    r3_texcoord_render(m, sh_blit_alpha.a_position, sh_blit_alpha.a_texcoord);
}

void r3_render_blit(const struct r3_mesh *m, unsigned int tex) {
    glUseProgram(sh_blit.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,tex);
    glUniform1i(sh_blit.u_sample, 0);

    r3_texcoord_render(m, sh_blit.a_position, sh_blit.a_texcoord);
}

void r3_render_blur_width(const struct r3_mesh *m, unsigned int tex, float aspect, float width) {
    glUseProgram(sh_gaussian.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,tex);
    glUniform1i(sh_gaussian.u_sample, 0);
    glUniform2f(sh_gaussian.u_offset, aspect / width, 0);
    
    r3_texcoord_render(m, sh_gaussian.a_position, sh_gaussian.a_texcoord);
}

void r3_render_blur_height(const struct r3_mesh *m, unsigned int tex, float aspect, float height) {
    glUseProgram(sh_gaussian.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,tex);
    glUniform1i(sh_gaussian.u_sample, 0);
    glUniform2f(sh_gaussian.u_offset, 0, aspect / height);
    
    r3_texcoord_render(m, sh_gaussian.a_position, sh_gaussian.a_texcoord);
}

void r3_render_high_pass(const struct r3_mesh *m, unsigned int tex) {
    glUseProgram(sh_high_pass.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,tex);
    glUniform1i(sh_high_pass.u_sample, 0);
    
    r3_texcoord_render(m, sh_high_pass.a_position, sh_high_pass.a_texcoord);
}

/*
void render_light(const struct r3_mesh *m, const struct r3_shader *sh, m4f mv, m4f mvp) {
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

unsigned int load_texture(char *data, int width, int height) {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

unsigned int r3_load_tga_rgb_texture(const char *path) {
    int width, height;
    char *data = r3_load_tga_rgb(path, &width, &height);
    if (data) {
        unsigned int tex = load_texture(data, width, height);
        free(data);
        return tex;
    }
    return 0;
}

unsigned int r3_load_tga_bgr_texture(const char *path) {
    int width, height;
    char *data = r3_load_tga_bgr(path, &width, &height);
    if (data) {
        unsigned int tex = load_texture(data, width, height);
        free(data);
        return tex;
    }
    return 0;
}

void r3_remove_mesh(const struct r3_mesh *m) {
    glDeleteBuffers(1, &m->vbo);
    glDeleteBuffers(1, &m->ibo);
}

void *r3_offset_ptr(enum r3_verts_tag tag, enum r3_vert vert) {
    return (void*)r3_offset(tag, vert);
}

ssize_t r3_offset(enum r3_verts_tag tag, enum r3_vert vert) {
    switch (tag) {
    case R3_VERTS_C:
        switch (vert) {
        case R3_POSITION:
            return 0;
        case R3_COLOR:
            return sizeof(v3f);
        default:
            assert(false);
        }
        break;
    case R3_VERTS_N:
        switch (vert) {
        case R3_POSITION:
            return 0;
        case R3_NORMAL:
            return sizeof(v3f);
        default:
            assert(false);
        }
        break;
    case R3_VERTS_T:
        switch (vert) {
        case R3_POSITION:
            return 0;
        case R3_TEXCOORD:
            return sizeof(v3f);
        default:
            assert(false);
        }
        break;
    case R3_VERTS_CN:
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
    case R3_VERTS_NT:
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
    case R3_VERTS_CNT:
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

const r3_mesh_t *r3_cube_mesh() {
    return &cube_mesh;
}

const r3_mesh_t *r3_quad_mesh() {
    return &quad_mesh;
}

void r3_quit() {
    r3_remove_mesh(&cube_mesh);
    r3_remove_mesh(&quad_mesh);
    glDeleteProgram(sh_cell.program);
    glDeleteProgram(sh_c.program);
    glDeleteProgram(sh_n.program);
    glDeleteProgram(sh_t.program);
    glDeleteProgram(sh_cn.program);
    glDeleteProgram(sh_ct.program);
    glDeleteProgram(sh_nt.program);
    glDeleteProgram(sh_cnt.program);
    glDeleteProgram(sh_blit.program);
    glDeleteProgram(sh_blit_alpha.program);
    glDeleteProgram(sh_gaussian.program);
    glDeleteProgram(sh_high_pass.program);
    glDeleteProgram(sh_light.program);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_DestroyWindow(window);
}
