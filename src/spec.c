#include "r3.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "r3_private.h"

struct r3_spec *r3_create_cuboid_spec() {
	const float colors[3*24] = {
		// Red
		1.0, 0.0, 0.0,
		0.6, 0.4, 0.0,
		0.6, 0.0, 0.4,
		0.6, 0.0, 0.0,
		// Cyan
		0.4, 0.6, 0.6,
		0.0, 1.0, 0.6,
		0.0, 0.6, 1.0,
		0.0, 0.6, 0.6,
		// Green
		0.4, 0.6, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.6, 0.4,
		0.0, 0.6, 0.0,
		// Magenta
		1.0, 0.0, 1.0,
		0.6, 0.4, 0.6,
		0.6, 0.0, 1.0,
		0.6, 0.0, 0.6,
		// Blue
		0.4, 0.0, 0.6,
		0.0, 0.4, 0.6,
		0.0, 0.0, 1.0,
		0.0, 0.0, 0.6,
		// Yellow
		1.0, 0.6, 0.0,
		0.6, 1.0, 0.0,
		0.6, 0.6, 0.4,
		0.6, 0.6, 0.0,
	};
	const float positions[3*24] = {
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f,  0.5f, 0.5f,
		0.5f,  0.5f, 0.5f, 
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
	};
	const float normals[3*24] = {
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};
	const float texcoords[2*24] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
	};
	const unsigned short int indices[36] = {
		0, 2, 1,
		0, 3, 2, 
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11, 
		12, 15, 14,
		12, 14, 13, 
		16, 17, 18,
		16, 18, 19, 
		20, 23, 22,
		20, 22, 21
	};

	char *buf = malloc(sizeof(r3_spec_t) + 24 * sizeof(r3_cnt_t) + 36 * sizeof(unsigned short int));
	struct r3_spec *spec = (void*)buf;
	spec->verts.tag = R3_VERTS_CNT;
	spec->verts.len = 24;
	spec->verts.data = (void*) (sizeof(struct r3_spec) + buf);
	spec->indices.tag = R3_INDICES_USHORT;
	spec->indices.len = 36;
	spec->indices.data = (void*)(24 * sizeof(r3_cnt_t) + sizeof(struct r3_spec) + buf);

	for (int i = 0; i < 24; i++) {
		spec->verts.cnt[i] = (r3_cnt_t) {
			.position = _v3f(positions[i*3+0], positions[i*3+1], positions[i*3+2]),
			.color = _v3f(colors[i*3+0], colors[i*3+1], colors[i*3+2]),
			.normal = _v3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]),
			.texcoord = _v2f(texcoords[i*2+0], texcoords[i*2+1]),
		};
	}

	memcpy(spec->indices.data, indices, sizeof(unsigned int short) * 36);

	return spec;
}
