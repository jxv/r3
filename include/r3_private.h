#ifndef R3_PRIVATE_H
#define R3_PRIVATE_H

unsigned int r3_make_shader(const char *src, unsigned int type, int src_len);
unsigned int r3_load_shader(const char *path, unsigned int type);
unsigned int r3_make_program(unsigned int vert_shader, unsigned int frag_shader);
unsigned int r3_make_program_from_src(const char *vert_src, int vert_src_len, const char *frag_src, int frag_src_len);
unsigned int r3_make_program_from_src_unsigned(const unsigned char *vsh, int vsh_len, const unsigned char *fsh, int fsh_len);
unsigned int r3_load_program_from_path(const char *vert_path, const char *frag_path);

void r3_load_shaders();

#endif
