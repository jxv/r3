#ifndef R3_H
#define R3_H

#include <ml.h>
#include <GLES2/gl2.h>

struct r3_ren {
	v2i window_size;
	GLenum clear_bits;
	v3f clear_color;
	void *backend;
	void (*render)(struct r3_ren *);
	void (*quit)(struct r3_ren *);
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
			v2f uv;	
		};
		float val[5];
	};
};

struct r3_pnt {
	union {
		struct {
			v3f position;
			v3f normal;
			v2f uv;
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
			v2f uv;
		};
		float val[11];
	};
};

enum r3_verts_tag {
	R3_VERTS_PC,
	R3_VERTS_PN,
	R3_VERTS_PCN,
	R3_VERTS_PT,
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
};

struct r3_indices {
	enum r3_indices_tag tag;
	union {
		void *data;
		unsigned short int *ushort;
	};
	unsigned int len;
};

struct r3_spec {
	struct r3_verts verts;
	struct r3_indices indices;
};

struct r3_mesh {
	unsigned int vbo;
	unsigned int ibo;
	unsigned int num_indices;
};

struct r3_shader {
	unsigned int program_id;
	int position_id;
	int color_id;
	int normal_id;
	int tex_coord_id;
	int proj_id;
	int mv_id;

	int normal_mat_id;
	int diffuse_color_id;
	int ambient_color_id;
	int specular_color_id;
	int shininess_id;
	int light_direction_id;
	int diffuse_map_id;
};

struct r3_resource {
	struct r3_mesh mesh;
	struct r3_shader shader;
	unsigned int tex_id;
};

void r3_clear(struct r3_ren *ren);
void r3_render(struct r3_ren *ren);
void r3_quit(struct r3_ren *ren);

char *r3_load_file(const char *path);
char* r3_load_tga(const char *fileName, int *width, int *height);
unsigned int r3_make_shader(const char *src, GLenum type);
unsigned int r3_load_shader(const char *path, GLenum type);
unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader);
unsigned int r3_make_program_from_src(const char *vert_src, const char *frag_src);
unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path);

static const GLenum R3_DEFAULT_CLEAR_BITS = GL_COLOR_BUFFER_BIT
					  | GL_DEPTH_BUFFER_BIT
					  | GL_STENCIL_BUFFER_BIT;
static const v3f R3_DEFAULT_CLEAR_COLOR = {.x = 0, .y = 0, .z = 0};

void r3_viewport(const struct r3_ren *ren);
void r3_enable_tests(const struct r3_ren *ren);
void r3_make_mesh_from_spec(const struct r3_spec *spec, struct r3_mesh *m);

#endif
