#ifndef R3_H
#define R3_H

#include <ml.h>

struct r3_mesh_spec {
	v3f *positions;
	v3f *colors;
	v3f *normals;
	v2f *uvs;
	unsigned int *indices[3];
	unsigned int size;
};

struct r3_mesh_data {
	v3f *vertices;
	unsigned int *indices;
	unsigned int size;
};

#endif
