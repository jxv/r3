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
	unsigned int tex;
};

struct bo {
	unsigned int fbo;
	unsigned int color;
	unsigned int depth;
	unsigned int stencil;
	unsigned int tex;
	v2i size;
};

#define COUNT 5

struct bo lf[COUNT];
struct bo rt[COUNT];

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
struct r3_shader blit, blit_alpha, blur, high_pass, light;

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
			.texcoord = _v2f(0, 1)
		},
		(struct r3_pt) {
			.position = _v3f(-1,-1, 1),
			.texcoord = _v2f(0, 0)
		},
		(struct r3_pt) {
			.position = _v3f( 1, 1, 1),
			.texcoord = _v2f(1, 1)
		},
		(struct r3_pt) {
			.position = _v3f( 1,-1, 1),
			.texcoord = _v2f(1, 0)
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
	glEnableVertexAttribArray(sh->attrib.position);
	glVertexAttribPointer(sh->attrib.position, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	if (flags == R3_TEXCOORD) {
		glEnableVertexAttribArray(sh->attrib.texcoord);
		glVertexAttribPointer(sh->attrib.texcoord, 2, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_TEXCOORD));
	}
	if (flags == R3_NORMAL) {
		glEnableVertexAttribArray(sh->attrib.normal);
		glVertexAttribPointer(sh->attrib.normal, 3, GL_FLOAT, GL_FALSE,
			r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(sh->attrib.position);
	if (flags == R3_TEXCOORD) {
		glDisableVertexAttribArray(sh->attrib.texcoord);
	}
	if (flags == R3_NORMAL) {
		glDisableVertexAttribArray(sh->attrib.normal);
	}
}

void make_blit()
{
	blit.program = r3_load_program_from_path("res/shader/blit.vert", "res/shader/blit.frag");
	glUseProgram(blit.program);
	blit.attrib.position = glGetAttribLocation(blit.program, "a_position");
	blit.attrib.texcoord = glGetAttribLocation(blit.program, "a_texcoord");
	blit.uniform.sample = glGetUniformLocation(blit.program, "u_sample");
}

void make_blit_alpha()
{
	blit_alpha.program = r3_load_program_from_path("res/shader/blit.vert", "res/shader/blit_alpha.frag");
	glUseProgram(blit_alpha.program);
	blit_alpha.attrib.position = glGetAttribLocation(blit_alpha.program, "a_position");
	blit_alpha.attrib.texcoord = glGetAttribLocation(blit_alpha.program, "a_texcoord");
	blit_alpha.uniform.sample = glGetUniformLocation(blit_alpha.program, "u_sample");
	blit_alpha.uniform.coefficients = glGetUniformLocation(blit_alpha.program, "u_alpha");
}

void render_blit_alpha(const struct r3_mesh *m, unsigned int tex, float alpha)
{
	glUseProgram(blit_alpha.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(blit_alpha.uniform.sample, 0);
	glUniform1f(blit_alpha.uniform.coefficients, alpha);
	render_drawable(m, &blit_alpha, R3_TEXCOORD);
}

void render_blit(const struct r3_mesh *m, unsigned int tex)
{
	glUseProgram(blit.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(blit.uniform.sample, 0);
	render_drawable(m, &blit, R3_TEXCOORD);
}

void make_blur()
{
	blur.program = r3_load_program_from_path("res/shader/blit.vert", "res/shader/gaussian.frag");
	glUseProgram(blur.program);
	blur.attrib.position = glGetAttribLocation(blur.program, "a_position");
	blur.attrib.texcoord = glGetAttribLocation(blur.program, "a_texcoord");
	blur.uniform.sample = glGetUniformLocation(blur.program, "u_sample");
	blur.uniform.coefficients = glGetUniformLocation(blur.program, "u_coefficients");
	blur.uniform.offset = glGetUniformLocation(blur.program, "u_offset");

	const float kernel[3] = { 5.0f / 16.0f, 6.0f / 16.0f, 5.0f / 16.0f };
	glUniform1fv(blur.uniform.coefficients, 3, kernel);
}

void render_blur(const struct r3_mesh *m, unsigned int tex, bool is_width, float len)
{
	glUseProgram(blur.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(blur.uniform.sample, 0);
	if (is_width) {
		const float offset = 1.2f / len;
		glUniform2f(blur.uniform.offset, offset, 0);
	} else {
		const float offset = 1.2f / len;
		glUniform2f(blur.uniform.offset, 0, offset);
	}
	render_drawable(m, &blur, R3_TEXCOORD);
}

void make_light()
{
	light.program = r3_load_program_from_path("res/shader/light.vert", "res/shader/light.frag");
	glUseProgram(light.program);
	light.attrib.position = glGetAttribLocation(light.program, "a_position");
	light.attrib.normal = glGetAttribLocation(light.program, "a_normal");
	light.uniform.mvp = glGetUniformLocation(light.program, "u_mvp");
	light.uniform.normal = glGetUniformLocation(light.program, "u_normal");
	light.uniform.light_position = glGetUniformLocation(light.program, "u_light_position");
	light.uniform.ambient = glGetUniformLocation(light.program, "u_ambient");
	light.uniform.diffuse = glGetUniformLocation(light.program, "u_diffuse");
	light.uniform.specular = glGetUniformLocation(light.program, "u_specular");
	light.uniform.shininess = glGetUniformLocation(light.program, "u_shininess");
}

void render_light(const struct r3_mesh *m, m4f mv, m4f mvp)
{
	glUseProgram(light.program);
	glUniformMatrix4fv(light.uniform.mvp, 1, GL_FALSE, mvp.val);
	glUniformMatrix3fv(light.uniform.normal, 1, GL_FALSE, m3m4f(mv).val);
	glUniform3fv(light.uniform.light_position, 1, light_pos.val);
	glUniform3fv(light.uniform.ambient, 1, ambient.val);
	glUniform3fv(light.uniform.diffuse, 1, diffuse.val);
	glUniform3fv(light.uniform.specular, 1, specular.val);
	glUniform1f(light.uniform.shininess, shininess);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glEnableVertexAttribArray(light.attrib.position);
	glEnableVertexAttribArray(light.attrib.normal);
	glVertexAttribPointer(light.attrib.position, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_POSITION));
	glVertexAttribPointer(light.attrib.normal, 3, GL_FLOAT, GL_FALSE, r3_verts_tag_sizeof(m->verts_tag), (void*)r3_offset(m->verts_tag, R3_NORMAL));
	glDrawElements(GL_TRIANGLES, m->num_indices, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(light.attrib.position);
	glDisableVertexAttribArray(light.attrib.normal);
}

void make_high_pass()
{
	high_pass.program = r3_load_program_from_path("res/shader/blit.vert", "res/shader/high_pass.frag");
	glUseProgram(high_pass.program);
	high_pass.attrib.position = glGetAttribLocation(high_pass.program, "a_position");
	high_pass.attrib.texcoord = glGetAttribLocation(high_pass.program, "a_texcoord");
	high_pass.uniform.sample = glGetUniformLocation(high_pass.program, "u_sample");
	high_pass.uniform.threshold = glGetUniformLocation(high_pass.program, "u_threshold");
	glUniform1f(high_pass.uniform.threshold, 0.65);
}

void render_high_pass(const struct r3_mesh *m, unsigned int tex)
{
	glUseProgram(high_pass.program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glUniform1i(blit.uniform.sample, 0);
	render_drawable(m, &high_pass, R3_TEXCOORD);
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
		on.tex = create_fbo_tex(on.size.x, on.size.y);

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
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, off.color);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off.depth);
		off.tex = create_fbo_tex(off.size.x, off.size.y);
		// check completion
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return EXIT_SUCCESS;
		}

		for (int w = on.size.x, h = on.size.y, i = 0; i < COUNT; i++, w >>= 1, h >>= 1) {
			glGenFramebuffers(1, &lf[i].fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
			lf[i].size = _v2i(w,h);
			glGenRenderbuffers(1, &lf[i].color);
			glBindRenderbuffer(GL_RENDERBUFFER, lf[i].color);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, lf[i].color);
			lf[i].tex = create_fbo_tex(w,h);
			
			glGenFramebuffers(1, &rt[i].fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
			rt[i].size = _v2i(w,h);
			glGenRenderbuffers(1, &rt[i].color);
			glBindRenderbuffer(GL_RENDERBUFFER, rt[i].color);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rt[i].color);
			rt[i].tex = create_fbo_tex(w,h);
		}
		
		//r3_make_normal_shader(&res.shader);
		r3_make_color_normal_texture_shader(&res.shader);
		struct r3_spec *spec = r3_create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &res.mesh);
		free(spec);
		res.tex = r3_load_tga_texture("res/img/base_map.tga");
		
		quad = make_quad();

		make_blit();
		make_blit_alpha();
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
			const m4f rot = rotm4f(angle, _v3f(0.9, 0.5, 0.1));
			mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3, 3, 3))), translate);
			mvp = mulm4f(persp, mv);
		}
		{ // Render
			// off screen
			glBindFramebuffer(GL_FRAMEBUFFER, off.fbo);
			glViewport(0, 0, off.size.x, off.size.y);
			glClearColor(1,1,1,1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_ALWAYS);
			render_blit(&quad, res.tex);
			glDepthFunc(GL_LESS);
			r3_render_color_normal_texture(&res.mesh, &res.shader, res.tex, mv, mvp, light_pos, ambient, specular, shininess);

			for (int i = 0; i < COUNT; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glDepthFunc(GL_ALWAYS);
				render_blit(&quad, off.tex);
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
				glViewport(0, 0, rt[i].size.x, rt[i].size.y);
				glDepthFunc(GL_ALWAYS);
				render_blit(&quad, lf[i].tex);
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				glDepthFunc(GL_ALWAYS);
				render_high_pass(&quad, rt[i].tex);
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
				glViewport(0, 0, rt[i].size.x, rt[i].size.y);
				glDepthFunc(GL_ALWAYS);
				render_blur(&quad, lf[i].tex, true, WINDOW_WIDTH);
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				glDepthFunc(GL_ALWAYS);
				render_blur(&quad, rt[i].tex, false, WINDOW_HEIGHT);
				glDepthFunc(GL_LESS);

				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				glDepthFunc(GL_ALWAYS);
				render_blit(&quad, rt[i].tex);
				glDepthFunc(GL_LESS);
			}
			
			glBindFramebuffer(GL_FRAMEBUFFER, on.fbo);
			glViewport(0, 0, on.size.x, on.size.y);
			glClearColor(0,0,0,1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDepthFunc(GL_ALWAYS);
			render_blit(&quad, res.tex);
			glDepthFunc(GL_LESS);
			r3_render_color_normal_texture(&res.mesh, &res.shader, res.tex, mv, mvp, light_pos, ambient, specular, shininess);

			glBlendFunc(GL_ONE, GL_ONE);
			glEnable(GL_BLEND);
			glDepthFunc(GL_ALWAYS);
			for (int i = 0; i < COUNT; i++) {
				render_blit_alpha(&quad, lf[i].tex, 0.5);
			}
			glDepthFunc(GL_LESS);			
			glDisable(GL_BLEND);
			
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
