#ifndef R3_H
#define R3_H

#include <ml.h>
#include <stdlib.h>

struct r3_ren {
	v2i window_size;
	unsigned int clear_bits;
	v3f clear_color;
	void *backend;
	void (*render)(struct r3_ren *);
	void (*quit)(struct r3_ren *);
};

typedef struct r3_ren r3_ren_t;

enum r3_vert {
	R3_POSITION,
	R3_COLOR,
	R3_NORMAL,
	R3_TEXCOORD,
};

typedef enum r3_vert r3_vert_t;

struct r3_pc {
	union {
		struct {
			v3f position;
			v3f color;
		};
		float val[6];
	};
};

typedef struct r3_pc r3_pc_t;

struct r3_pn {
	union {
		struct {
			v3f position;
			v3f normal;
		};
		float val[6];
	};
};

typedef struct r3_pn r3_pn_t;

struct r3_pcn {
	union {
		struct {
			v3f position;
			v3f color;
			v3f normal;
		};
		float val[9];
	};
};

typedef struct r3_pcn r3_pcn_t;

struct r3_pt {
	union {
		struct {
			v3f position;
			v2f texcoord;	
		};
		float val[5];
	};
};

typedef struct r3_pt r3_pt_t;

struct r3_pnt {
	union {
		struct {
			v3f position;
			v3f normal;
			v2f texcoord;
		};
		float val[8];
	};
};

typedef struct r3_pnt r3_pnt_t;

struct r3_pcnt {
	union {
		struct {
			v3f position;
			v3f color;
			v3f normal;
			v2f texcoord;
		};
		float val[11];
	};
};

typedef struct r3_pcnt r3_pcnt_t;

enum r3_verts_tag {
	R3_VERTS_PC,
	R3_VERTS_PN,
	R3_VERTS_PT,
	R3_VERTS_PCN,
	R3_VERTS_PNT,
	R3_VERTS_PCNT,
};

typedef enum r3_verts_tag r3_verts_tag_t;

struct r3_verts {
	enum r3_verts_tag tag;
	union {
		float *data;
		struct r3_pc *pc;
		struct r3_pn *pn;
		struct r3_pcn *pcn;
		struct r3_pt *pt;
		struct r3_pnt *pnt;
		struct r3_pcnt *pcnt;
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

struct r3_shader_attrib {
	int position;
	int normal;
	int texcoord;
	int color;
};

typedef struct r3_shader_attrib r3_shader_attrib_t;

struct r3_shader_uniform {
	int mvp;
	int normal;
	int light_position;
	int ambient;
	int diffuse;
	int specular;
	int shininess;
	int sample;
	int coefficients;
	int offset;
	int threshold;
	int alpha;
};

typedef struct r3_shader_uniform r3_shader_uniform_t;

struct r3_shader {
	unsigned int program;
	struct r3_shader_attrib attrib;
	struct r3_shader_uniform uniform;
};

typedef struct r3_shader r3_shader_t;

void r3_clear(r3_ren_t *ren);
void r3_render(r3_ren_t *ren);
void r3_quit(r3_ren_t *ren);

unsigned int r3_load_tga_texture(const char *path);
char* r3_load_tga(const char *fileName, int *width, int *height);
unsigned int r3_make_shader(const char *src, unsigned int type, int src_len);
unsigned int r3_load_shader(const char *path, unsigned int type);
unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader);
unsigned int r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len);
unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path);

void r3_viewport(const r3_ren_t *ren);
void r3_enable_tests(const r3_ren_t *ren);
unsigned int r3_make_fbo_tex(int width, int height);
void r3_make_mesh_from_spec(const r3_spec_t *spec, r3_mesh_t *m);
r3_mesh_t r3_make_quad();

r3_spec_t *r3_create_cuboid_spec();

void r3_render_blit_alpha(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex, float alpha);
void r3_render_blit(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex);
void r3_render_blur_width(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex, float aspect, float width);
void r3_render_blur_height(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex, float aspect, float height);
void r3_render_high_pass(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex);
void r3_render_normal(const r3_mesh_t *m, const r3_shader_t *sh, m4f mv, m4f mvp,
	v3f light_position, v3f ambient, v3f diffuse, v3f specular, float shininess);
void r3_render_texture(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex, m4f mvp);
void r3_render_color(const r3_mesh_t *m, const r3_shader_t *sh, m4f mvp);
void r3_render_color_normal_texture(const r3_mesh_t *m, const r3_shader_t *sh, unsigned int tex,
	m4f mv, m4f mvp, v3f light_position, v3f ambient_material, v3f specular_material, float shininess);
	
void r3_make_cell_shader(r3_shader_t *sh);
void r3_make_normal_shader(r3_shader_t *sh);
void r3_make_texture_shader(r3_shader_t *sh);
void r3_make_color_shader(r3_shader_t *sh);
void r3_make_color_normal_texture_shader(r3_shader_t *sh);
void r3_make_blit_shader(r3_shader_t *sh);
void r3_make_blit_alpha_shader(r3_shader_t *sh);
void r3_make_blur_shader(r3_shader_t *sh);
void r3_make_high_pass_shader(r3_shader_t *sh);
void r3_make_light_shader(r3_shader_t *sh);

void r3_break_mesh(const r3_mesh_t *m);
void r3_break_shader(const r3_shader_t *sh);

ssize_t r3_verts_tag_sizeof(r3_verts_tag_t tag);
ssize_t r3_verts_sizeof(const r3_verts_t *verts);
ssize_t r3_indices_tag_sizeof(r3_indices_tag_t tag);
ssize_t r3_indices_sizeof(const r3_indices_t *indices);

ssize_t r3_offset(r3_verts_tag_t tag, r3_vert_t vert);
void *r3_offset_ptr(r3_verts_tag_t tag, r3_vert_t vert);

#endif
