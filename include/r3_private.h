#ifndef R3_PRIVATE_H
#define R3_PRIVATE_H

#include <SDL2/SDL.h>
#include <GLES2/gl2.h>
#include <SDL2/SDL_opengles2.h>

unsigned int r3_make_shader(const char *src, unsigned int type, int src_len);
unsigned int r3_load_shader(const char *path, unsigned int type);
unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader);
unsigned int r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len);
unsigned int r3_make_program_from_src_unsigned(const unsigned char *vsh, int vsh_len, const unsigned char *fsh, int fsh_len);
unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path);

void r3_load_shaders();
r3_spec_t *r3_create_cuboid_spec();

void r3_make_cube();
void r3_make_quad();

extern SDL_Window *window;
extern v2i window_dim;
extern SDL_GLContext context;
extern struct r3_mesh mesh_cube, mesh_quad;

#endif
