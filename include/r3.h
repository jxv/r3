#ifndef R3_H
#define R3_H

#include <ml.h>
#include <stdlib.h>
#include <GLES2/gl2.h>

bool r3_init(const char *title, v2i dim);
void r3_clear(v3f color, unsigned int clear_bits);
void r3_render();
void r3_quit();

// AUX API
unsigned int r3_load_tga_texture(const char *path);
char* r3_load_tga(const char *fileName, int *width, int *height);

//

enum r3_vert {
	R3_POSITION,
	R3_COLOR,
	R3_NORMAL,
	R3_TEXCOORD,
};

typedef enum r3_vert r3_vert_t;

typedef struct r3_c {
	union {
		struct {
			v3f position;
			v3f color;
		};
		float val[6];
	};
} r3_c_t;

typedef struct r3_n {
	union {
		struct {
			v3f position;
			v3f normal;
		};
		float val[6];
	};
} r3_n_t;

typedef struct r3_cn {
	union {
		struct {
			v3f position;
			v3f color;
			v3f normal;
		};
		float val[9];
	};
} r3_cn_t;

typedef struct r3_t {
	union {
		struct {
			v3f position;
			v2f texcoord;	
		};
		float val[5];
	};
} r3_t_t;

typedef struct r3_nt {
	union {
		struct {
			v3f position;
			v3f normal;
			v2f texcoord;
		};
		float val[8];
	};
} r3_nt_t;

typedef struct r3_cnt {
	union {
		struct {
			v3f position;
			v3f color;
			v3f normal;
			v2f texcoord;
		};
		float val[11];
	};
} r3_cnt_t;

enum r3_verts_tag {
	R3_VERTS_C,
	R3_VERTS_N,
	R3_VERTS_T,
	R3_VERTS_CN,
	R3_VERTS_NT,
	R3_VERTS_CNT,
};

typedef enum r3_verts_tag r3_verts_tag_t;

struct r3_verts {
	enum r3_verts_tag tag;
	union {
		float *data;
		struct r3_c *c;
		struct r3_n *n;
		struct r3_cn *cn;
		struct r3_t *t;
		struct r3_nt *nt;
		struct r3_cnt *cnt;
	};
	unsigned int len;
};

typedef struct r3_verts r3_verts_t;

enum r3_indices_tag {
	R3_INDICES_USHORT,
	R3_INDICES_UINT,
};

typedef enum r3_indices_tag r3_indices_tag_t;

struct r3_indices {
	enum r3_indices_tag tag;
	union {
		void *data;
		unsigned short int *ushort;
		unsigned int *uint;
	};
	unsigned int len;
};

typedef struct r3_indices r3_indices_t;

struct r3_spec {
	struct r3_verts verts;
	struct r3_indices indices;
};

typedef struct r3_spec r3_spec_t;

struct r3_mesh {
	enum r3_verts_tag verts_tag;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int num_indices;
};

typedef struct r3_mesh r3_mesh_t;

struct r3_normal {
    m3f mv;
    v3f light_position;
    v3f ambient_color;
    v3f diffuse_color;
    v3f specular_color;
    float shininess;
};

typedef struct r3_normal r3_normal_t;

void r3_viewport();
void r3_enable_tests();

unsigned int r3_make_fbo_tex(int width, int height);
void r3_make_mesh_from_spec(const r3_spec_t *spec, r3_mesh_t *m);

const r3_mesh_t *r3_cube_mesh();
const r3_mesh_t *r3_quad_mesh();

void r3_render_range_c(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp);
void r3_render_range_n(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, const r3_normal_t *);
void r3_render_range_cn(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, const r3_normal_t *);
void r3_render_range_t(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, unsigned int tex);
void r3_render_range_ct(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, unsigned int tex);
void r3_render_range_nt(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, const r3_normal_t *, unsigned int tex);
void r3_render_range_cnt(const r3_mesh_t *m, int start_idx, int end_idx, m4f mvp, const r3_normal_t *, unsigned int tex);

void r3_render_c(const r3_mesh_t *m, m4f mvp);
void r3_render_n(const r3_mesh_t *m, m4f mvp, const r3_normal_t *);
void r3_render_cn(const r3_mesh_t *m, m4f mvp, const r3_normal_t *);
void r3_render_t(const r3_mesh_t *m, m4f mvp, unsigned int tex);
void r3_render_ct(const r3_mesh_t *m, m4f mvp, unsigned int tex);
void r3_render_nt(const r3_mesh_t *m, m4f mvp, const r3_normal_t *, unsigned int tex);
void r3_render_cnt(const r3_mesh_t *m, m4f mvp, const r3_normal_t *, unsigned int tex);

void r3_render_blit_alpha(const r3_mesh_t *m,  unsigned int tex, float alpha);
void r3_render_blit(const r3_mesh_t *m,  unsigned int tex);
void r3_render_blur_width(const r3_mesh_t *m,  unsigned int tex, float aspect, float width);
void r3_render_blur_height(const r3_mesh_t *m,  unsigned int tex, float aspect, float height);
void r3_render_high_pass(const r3_mesh_t *m,  unsigned int tex);

void r3_remove_mesh(const r3_mesh_t *m);

ssize_t r3_verts_tag_sizeof(r3_verts_tag_t tag);
ssize_t r3_verts_sizeof(const r3_verts_t *verts);
ssize_t r3_indices_tag_sizeof(r3_indices_tag_t tag);
ssize_t r3_indices_sizeof(const r3_indices_t *indices);

ssize_t r3_offset(r3_verts_tag_t tag, r3_vert_t vert);
void *r3_offset_ptr(r3_verts_tag_t tag, r3_vert_t vert);

#endif
