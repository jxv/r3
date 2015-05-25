#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ml.h>
#include <r3_sdl.h>
#include <GLES2/gl2.h>

#ifdef GCW0
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
#else
#define WINDOW_WIDTH (320*2)
#define WINDOW_HEIGHT (240*2)
#endif

struct res {
	struct r3_mesh mesh;
	struct r3_shader shader;
	unsigned int tex_id;
};

struct bo {
	unsigned int fbo;
	unsigned int color;
	unsigned int depth;
	unsigned int stencil;
	unsigned int tex_id;
	v2i size;
};

struct res res;
float dt = 1 / 60.0;
float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
const v3f light_pos = {0.25, 0.25, 1};
const v3f ambient = {0.05, 0.05, 0.05};
const v3f diffuse = {0.75, 0.75, 0.75};
const v3f specular = {0.5, 0.5, 0.5};
float shininess = 100;
struct r3_ren ren;
struct res res;
struct r3_mesh quad;
m4f mv, mvp;
float angle = 0;
struct bo on, off;
struct r3_shader blit, blur, high_pass, light;

unsigned int create_fbo_tex(int width, int height)
{
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	return tex;
}

struct r3_mesh make_quad()
{
	struct r3_mesh m;
	const struct r3_pt verts[4] = {
		(struct r3_pt) {
			.position = _v3f(-1, 1, 1),
			.texcoord = _v2f(0, 0)
		},
		(struct r3_pt) {
			.position = _v3f(-1,-1, 1),
			.texcoord = _v2f(0, 1)
		},
		(struct r3_pt) {
			.position = _v3f( 1, 1, 1),
			.texcoord = _v2f(1, 0)
		},
		(struct r3_pt) {
			.position = _v3f( 1,-1, 1),
			.texcoord = _v2f(1, 1)
		},
	};
	const unsigned short int indices[6] = {
		0, 1, 2,
		1, 3, 2,
	};
	m.num_indices = 6;
	glGenBuffers(1, &m.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(struct r3_pt), verts, GL_STATIC_DRAW);
	glGenBuffers(1, &m.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.num_indices * sizeof(unsigned short int), indices, GL_STATIC_DRAW);
	m.verts_tag = R3_VERTS_PT;
	return m;
}


void render_drawable(const struct r3_mesh *m, const struct r3_shader *sh, int flags)
{
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glEnableVertexAttribArray(sh->attrib.position_id);
	glVertexAttribPointer(sh->attrib.position_id, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	if (flags == R3_TEXCOORD) {
		glEnableVertexAttribArray(sh->attrib.texcoord_id);
		glVertexAttribPointer(sh->attrib.texcoord_id, 2, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
	}
	if (flags == R3_NORMAL) {
		glEnableVertexAttribArray(sh->attrib.normal_id);
		glVertexAttribPointer(sh->attrib.normal_id, 3, GL_FLOAT, GL_FALSE,
			r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(sh->attrib.position_id);
	if (flags == R3_TEXCOORD) {
		glDisableVertexAttribArray(sh->attrib.texcoord_id);
	}
	if (flags == R3_NORMAL) {
		glDisableVertexAttribArray(sh->attrib.normal_id);
	}
}

void make_blit()
{
	blit.program_id = r3_load_program_from_path("res/shader/blit.vert", "res/shader/blit.frag");
	glUseProgram(blit.program_id);
	blit.attrib.position_id = glGetAttribLocation(blit.program_id, "a_position");
	blit.attrib.texcoord_id = glGetAttribLocation(blit.program_id, "a_texcoord");
	blit.uniform.sample_id = glGetUniformLocation(blit.program_id, "u_sample");
}

void render_blit(const struct r3_mesh *m, unsigned int tex_id)
{
	glUseProgram(blit.program_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex_id);
	glUniform1i(blit.uniform.sample_id, 0);
	render_drawable(m, &blit, R3_TEXCOORD);
}

void make_blur()
{
	blur.program_id = r3_load_program_from_path("res/shader/blit.vert", "res/shader/gaussian.frag");
	glUseProgram(blur.program_id);
	blur.attrib.position_id = glGetAttribLocation(blur.program_id, "a_position");
	blur.attrib.texcoord_id = glGetAttribLocation(blur.program_id, "a_texcoord");
	blur.uniform.sample_id = glGetUniformLocation(blur.program_id, "u_sample");
	blur.uniform.coefficients_id = glGetUniformLocation(blur.program_id, "u_coefficients");
	blur.uniform.offset_id = glGetUniformLocation(blur.program_id, "u_offset");
}

void render_blur(const struct r3_mesh *m, unsigned int tex_id)
{
	glUseProgram(blur.program_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex_id);
	glUniform1i(blur.uniform.sample_id, 0);
	
	render_drawable(m, &blur, R3_TEXCOORD);
}

void make_light()
{
	light.program_id = r3_load_program_from_path("res/shader/light.vert", "res/shader/light.frag");
	glUseProgram(light.program_id);
	light.attrib.position_id = glGetAttribLocation(light.program_id, "a_position");
	light.attrib.normal_id = glGetAttribLocation(light.program_id, "a_normal");
	light.uniform.mvp_id = glGetUniformLocation(light.program_id, "u_mvp");
	light.uniform.normal_id = glGetUniformLocation(light.program_id, "u_normal");
	light.uniform.light_position_id = glGetUniformLocation(light.program_id, "u_light_position");
	light.uniform.ambient_id = glGetUniformLocation(light.program_id, "u_ambient");
	light.uniform.diffuse_id = glGetUniformLocation(light.program_id, "u_diffuse");
	light.uniform.specular_id = glGetUniformLocation(light.program_id, "u_specular");
	light.uniform.shininess_id = glGetUniformLocation(light.program_id, "u_shininess");
}

void draw_light(const struct r3_mesh *m, m4f mv, m4f mvp)
{
	glUseProgram(light.program_id);
	glUniformMatrix4fv(light.uniform.mvp_id, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(light.uniform.normal_id, 1, GL_FALSE, m3m4f(mv).val);
	glUniform3fv(light.uniform.light_position_id, 1, light_pos.val);
	glUniform3fv(light.uniform.ambient_id, 1, ambient.val);
	glUniform3fv(light.uniform.diffuse_id, 1, diffuse.val);
	glUniform3fv(light.uniform.specular_id, 1, specular.val);
	glUniform1f(light.uniform.shininess_id, shininess);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glEnableVertexAttribArray(light.attrib.position_id);
	glEnableVertexAttribArray(light.attrib.normal_id);
	glVertexAttribPointer(light.attrib.position_id, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glVertexAttribPointer(light.attrib.normal_id, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(light.attrib.position_id);
	glDisableVertexAttribArray(light.attrib.normal_id);
}

void make_high_pass()
{
	high_pass.program_id = r3_load_program_from_path("res/shader/blit.vert", "res/shader/high_pass.frag");
	glUseProgram(high_pass.program_id);
	high_pass.attrib.position_id = glGetAttribLocation(high_pass.program_id, "a_position");
	high_pass.attrib.texcoord_id = glGetAttribLocation(high_pass.program_id, "a_texcoord");
	high_pass.uniform.sample_id = glGetUniformLocation(high_pass.program_id, "u_sample");
	high_pass.uniform.threshold_id = glGetUniformLocation(high_pass.program_id, "u_threshold");
}

int main(int argc, char *argv[])
{
	{ // Init
		r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);

		// defualt screen fb
		on.fbo = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, on.fbo);
		on.size = _v2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		// rb that will serve as the color attachment for the framebuffer.
		glGenRenderbuffers(1, &on.color);
		glBindRenderbuffer(GL_RENDERBUFFER, on.color);
		// Build the framebuffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, on.color);
		glBindRenderbuffer(GL_RENDERBUFFER, on.color);
		on.tex_id = create_fbo_tex(on.size.x, on.size.y);

		// fb for off-screen
		glGenFramebuffers(1, &off.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, off.fbo);
		off.size = _v2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		// rb for depth and color attachment for fb
		glGenRenderbuffers(1, &off.color);
		glBindRenderbuffer(GL_RENDERBUFFER, off.color);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, off.size.x, off.size.y);
		glGenRenderbuffers(1, &off.depth);
		glBindRenderbuffer(GL_RENDERBUFFER, off.depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, off.size.x, off.size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off.depth);
		off.tex_id = create_fbo_tex(off.size.x, off.size.y);
		// check completion
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return EXIT_SUCCESS;
		}
		
		//r3_make_normal_shader(&res.shader);
		r3_make_color_normal_texture_shader(&res.shader);
		struct r3_spec *spec = r3_create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &res.mesh);
		free(spec);
		res.tex_id = r3_load_tga_texture("res/img/base_map.tga");
		
		quad = make_quad();

		make_blit();
		make_blur();
		make_light();
		make_high_pass();

		r3_enable_tests(&ren);
	}
	bool done = false;
	while (!done) { // Main loop
		{ // Input
			SDL_Event event;
			while (SDL_PollEvent(&event) && !done) {
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
				}
			}
		}
		{ // Update
			angle = fmodf(angle + dt * 2, M_PI * 2);
			const m4f persp = perspf(45, aspect, 1, 20);
			const m4f translate = translatef(_v3f(0,0,-7));
			const m4f rot = rotm4f(angle, _v3f(0.9,0.5,0.2));
			mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
			mvp = mulm4f(persp, mv);
		}
		{ // Render
			// off screen
			glBindFramebuffer(GL_FRAMEBUFFER, off.fbo);
			glViewport(0,0,off.size.x,off.size.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_ALWAYS);
			render_blit(&quad, res.tex_id);
			glDepthFunc(GL_LESS);
			// r3_render_normal(&res.mesh, &res.shader, mv, mvp, light_pos, ambient, diffuse, specular, shininess);
			r3_render_color_normal_texture(&res.mesh, &res.shader, res.tex_id, mv, mvp, light_pos, ambient, specular, shininess);
			
			glBindFramebuffer(GL_FRAMEBUFFER, on.fbo);
			glViewport(0,0,on.size.x, on.size.y);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_ALWAYS);
			render_blit(&quad, off.tex_id);
			glDepthFunc(GL_LESS);
			
			r3_render(&ren);
		}
		SDL_Delay(16);
	}
	// Clean up
	r3_break_mesh(&res.mesh);
	r3_break_shader(&res.shader);
	r3_quit(&ren);
	return EXIT_SUCCESS;
}
