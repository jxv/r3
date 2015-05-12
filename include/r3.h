#ifndef R3_H
#define R3_H

#include <ml.h>

struct r3_mesh_point {
	union {
		struct {
			v3f position;	// 3
			v3f color;	// 6 = 3 + 3
			v3f normal;	// 9 = 6 + 3
			v2f uv;		// 11 = 9 + 2
		};
		float val[11];
	};
};

struct r3_mesh_spec {
	struct r3_mesh_point *points;
	unsigned int num_points;
	v3ui *indices;
	unsigned int num_indices;
};

 struct r3_mesh {
	unsigned int vbo;
	unsigned int ibo;
	unsigned int vao; // References to all data in mesh and describes its layout
	int index_count;
 };

struct r3_shader {
	unsigned int program_id;
	unsigned int position_id;
	unsigned int color_id;
	unsigned int normal_id;
	unsigned int uv_id;
	unsigned int pvm_mat_id;
	unsigned int vm_mat_id;
	unsigned int normal_mat_id;
	unsigned int diffused_color_id;
	unsigned int ambient_color_id;
	unsigned int specular_color_id;
	unsigned int shininess_id;
	unsigned int light_direction_id;
	unsigned int diffuse_map_id;
};

struct r3_resource {
	struct r3_mesh mesh;
	unsigned int texture_id;
	struct r3_shader shader;
};

#endif
