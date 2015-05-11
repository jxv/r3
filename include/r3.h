#ifndef R3_H
#define R3_H

#include <ml.h>

struct r3_mesh_spec {
	v3f *positions;
	v3f *colors;
	v3f *normals;
	v2f *uvs;
	unsigned int num_vertices;
	unsigned int *indices[3];
	unsigned int num_indices;
};

struct r3_mesh_data {
	v3f *vertices;
	unsigned int num_vertices;
	unsigned int *indices;
	unsigned int num_indices;
};

 struct r3_mesh {
	unsigned int vbo;
	unsigned int ibo;
	unsigned int vao;
	int index_count;
 };

struct r3_shader {
	unsigned int program_id;
	unsigned int position_id;
	unsigned int color_id;
	unsigned int uv_id;
	unsigned int mvp_id;
	unsigned int mv_id;
	unsigned int normal_id;
	unsigned int diffused_color_id;
	unsigned int ambient_color_id;
	unsigned int specular_color_id;
	unsigned int shininess_id;
	unsigned int light_direction_id;
	unsigned int diffuse_map_id;
};

#endif
