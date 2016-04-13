#ifndef R3_H
#define R3_H

#include <ml.h>
#include <stdlib.h>
#include <GLES2/gl2.h>

typedef enum {
    R3_CLEAR_BIT_COLOR = GL_COLOR_BUFFER_BIT,
    R3_CLEAR_BIT_DEPTH = GL_DEPTH_BUFFER_BIT,
    R3_CLEAR_BIT_STENCIL = GL_STENCIL_BUFFER_BIT,
} r3ClearBit;

typedef enum {
    R3_POSITION,
    R3_COLOR,
    R3_NORMAL,
    R3_TEXCOORD,
} r3Vert;

typedef struct {
    union {
        struct {
            v3f position;
            v3f color;
        };
        float val[6];
    };
} r3C;

typedef struct {
    union {
        struct {
            v3f position;
            v3f normal;
        };
        float val[6];
    };
} r3N;

typedef struct {
    union {
        struct {
            v3f position;
            v3f color;
            v3f normal;
        };
        float val[9];
    };
} r3CN;

typedef struct {
    union {
        struct {
            v3f position;
            v2f texcoord;
        };
        float val[5];
    };
} r3T;

typedef struct {
    union {
        struct {
            v3f position;
            v3f normal;
            v2f texcoord;
        };
        float val[8];
    };
} r3NT;

typedef struct {
    union {
        struct {
            v3f position;
            v3f color;
            v3f normal;
            v2f texcoord;
        };
        float val[11];
    };
} r3CNT;

typedef enum {
    R3_VERTS_C,
    R3_VERTS_N,
    R3_VERTS_T,
    R3_VERTS_CN,
    R3_VERTS_NT,
    R3_VERTS_CNT,
} r3VertsTag;

typedef struct {
    r3VertsTag tag;
    union {
        float *data;
	r3C *c;
	r3N *n;
	r3CN *cn;
	r3T *t;
	r3NT *nt;
	r3CNT *cnt;
    };
    unsigned int len;
} r3Verts;

typedef enum {
    R3_INDICES_USHORT,
    R3_INDICES_UINT,
} r3IndicesTag;

typedef struct {
    r3IndicesTag tag;
    union {
        void *data;
        unsigned short int *ushort;
        unsigned int *uint;
    };
    unsigned int len;
} r3Indices;

typedef struct {
    r3Verts verts;
    r3Indices indices;
} r3Spec;

typedef struct {
    r3VertsTag verts_tag;
    unsigned int vbo;
    unsigned int ibo;
    unsigned int num_indices;
} r3Mesh;

typedef struct {
    m3f mv;
    v3f light_position;
    v3f ambient_color;
    v3f diffuse_color;
    v3f specular_color;
    float shininess;
} r3Normal;

//

bool r3_init(const char *title, int w, int h);
void r3_clear(const v3f *color, r3ClearBit bits);
void r3_render();
void r3_quit();

// AUX API
unsigned int r3_load_tga_bgr_texture(const char *path);
unsigned int r3_load_tga_rgb_texture(const char *path);
char* r3_load_tga_bgr(const char *file_name, int *width, int *height);
char* r3_load_tga_rgb(const char *file_name, int *width, int *height);

//

void r3_viewport();
void r3_enable_tests();

unsigned int r3_make_fbo_tex(int width, int height);
void r3_make_mesh_from_spec(const r3Spec *spec, r3Mesh *m);

const r3Mesh *r3_cube_mesh();
const r3Mesh *r3_quad_mesh();

void r3_render_range_c(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp);
void r3_render_range_k(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const v3f *kolor);
void r3_render_range_n(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const r3Normal *);
void r3_render_range_cn(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const r3Normal *);
void r3_render_range_kn(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const v3f *kolor, const r3Normal *);
void r3_render_range_t(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, unsigned int tex);
void r3_render_range_ct(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, unsigned int tex);
void r3_render_range_kt(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const v3f *kolor, unsigned int tex);
void r3_render_range_nt(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const r3Normal *, unsigned int tex);
void r3_render_range_cnt(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const r3Normal *, unsigned int tex);
void r3_render_range_knt(const r3Mesh *m, int start_idx, int end_idx, const m4f *mvp, const v3f *kolor, const r3Normal *, unsigned int tex);

void r3_render_c(const r3Mesh *m, const m4f *mvp);
void r3_render_k(const r3Mesh *m, const m4f *mvp, const v3f *kolor);
void r3_render_n(const r3Mesh *m, const m4f *mvp, const r3Normal *);
void r3_render_cn(const r3Mesh *m, const m4f *mvp, const r3Normal *);
void r3_render_kn(const r3Mesh *m, const m4f *mvp, const v3f *kolor, const r3Normal *);
void r3_render_t(const r3Mesh *m, const m4f *mvp, unsigned int tex);
void r3_render_ct(const r3Mesh *m, const m4f *mvp, unsigned int tex);
void r3_render_kt(const r3Mesh *m, const m4f *mvp, const v3f *kolor, unsigned int tex);
void r3_render_nt(const r3Mesh *m, const m4f *mvp, const r3Normal *, unsigned int tex);
void r3_render_cnt(const r3Mesh *m, const m4f *mvp, const r3Normal *, unsigned int tex);
void r3_render_knt(const r3Mesh *m, const m4f *mvp, const v3f *kolor, const r3Normal *, unsigned int tex);

void r3_render_blit_alpha(const r3Mesh *m,  unsigned int tex, float alpha);
void r3_render_blit(const r3Mesh *m,  unsigned int tex);
void r3_render_blur_width(const r3Mesh *m,  unsigned int tex, float aspect, float width);
void r3_render_blur_height(const r3Mesh *m,  unsigned int tex, float aspect, float height);
void r3_render_high_pass(const r3Mesh *m,  unsigned int tex);

void r3_remove_mesh(const r3Mesh *m);

ssize_t r3Vertsag_sizeof(r3VertsTag tag);
ssize_t r3_verts_sizeof(const r3Verts *verts);
ssize_t r3Indicesag_sizeof(r3IndicesTag tag);
ssize_t r3_indices_sizeof(const r3Indices *indices);

ssize_t r3_offset(r3VertsTag tag, r3Vert vert);
void *r3_offset_ptr(r3VertsTag tag, r3Vert vert);

#endif
