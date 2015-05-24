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

enum r3_vert {
	R3_POSITION,
	R3_COLOR,
	R3_NORMAL,
	R3_TEXCOORD,
};

struct r3_pc {
	union {
		struct {
			v3f position;
			v3f color;
		};
		float val[6];
	};
};

struct r3_pn {
	union {
		struct {
			v3f position;
			v3f normal;
		};
		float val[6];
	};
};

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

struct r3_pt {
	union {
		struct {
			v3f position;
			v2f texcoord;	
		};
		float val[5];
	};
};

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

enum r3_verts_tag {
	R3_VERTS_PC,
	R3_VERTS_PN,
	R3_VERTS_PT,
	R3_VERTS_PCN,
	R3_VERTS_PNT,
	R3_VERTS_PCNT,
};

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

enum r3_indices_tag {
	R3_INDICES_USHORT,
	R3_INDICES_UINT,
};

struct r3_indices {
	enum r3_indices_tag tag;
	union {
		void *data;
		unsigned short int *ushort;
		unsigned int *uint;
	};
	unsigned int len;
};

struct r3_spec {
	struct r3_verts verts;
	struct r3_indices indices;
};

struct r3_mesh {
	enum r3_verts_tag verts_tag;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int num_indices;
};

struct r3_shader {
	unsigned int program_id;
	struct {
		int position_id;
		int normal_id;
		int texcoord_id;
		int color_id;
	} attrib;
	struct {
		int mvp_id;
		int normal_id;
		int light_position_id;
		int ambient_material_id;
		int diffuse_material_id;
		int specular_material_id;
		int shininess_id;
		int sample_id;
		int coefficients_id;
		int offset_id;
		int threshold_id;
	} uniform;
};

void r3_clear(struct r3_ren *ren);
void r3_render(struct r3_ren *ren);
void r3_quit(struct r3_ren *ren);

unsigned int r3_load_tga_texture(const char *path);
char* r3_load_tga(const char *fileName, int *width, int *height);
unsigned int r3_make_shader(const char *src, unsigned int type, int src_len);
unsigned int r3_load_shader(const char *path, unsigned int type);
unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader);
unsigned int r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len);
unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path);

void r3_viewport(const struct r3_ren *ren);
void r3_enable_tests(const struct r3_ren *ren);
void r3_make_mesh_from_spec(const struct r3_spec *spec, struct r3_mesh *m);

struct r3_spec *r3_create_cuboid_spec();

void r3_render_normal(const struct r3_mesh *m, const struct r3_shader *sh, unsigned int tex_id,
	m4f mv, m4f mvp, v3f light_position, v3f ambient_material, v3f specular_material, float shininess);
void r3_render_texture(const struct r3_mesh *m, const struct r3_shader *sh, unsigned int tex_id, m4f mvp);
void r3_render_color(const struct r3_mesh *m, const struct r3_shader *sh, m4f mvp);

void r3_make_cell_shader(struct r3_shader *sh);
void r3_make_normal_shader(struct r3_shader *sh);
void r3_make_texture_shader(struct r3_shader *sh);
void r3_make_color_shader(struct r3_shader *sh);

void r3_break_mesh(const struct r3_mesh *m);
void r3_break_shader(const struct r3_shader *sh);

ssize_t r3_verts_tag_sizeof(enum r3_verts_tag tag);
ssize_t r3_verts_sizeof(const struct r3_verts *verts);
ssize_t r3_indices_tag_sizeof(enum r3_indices_tag tag);
ssize_t r3_indices_sizeof(const struct r3_indices *indices);

ssize_t r3_offset(enum r3_verts_tag tag, enum r3_vert vert);

#endif
