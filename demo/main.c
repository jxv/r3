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

enum res_mode {
	RES_NORMAL = 0,
	RES_COLOR,
	RES_TEXTURE,
	RES_MODE_COUNT
};

struct res {
	enum res_mode mode;
	struct r3_mesh mesh;
	struct r3_shader shader[RES_MODE_COUNT];
	unsigned int tex_id;
};

#define COUNT 5

struct fb {
	unsigned int on;
	unsigned int scene;
	unsigned int off_lf[COUNT];
	unsigned int off_rt[COUNT];
};

struct rb {
	unsigned int on;
	unsigned int depth;
	unsigned int color;
	unsigned int off_lf[COUNT];
	unsigned int off_rt[COUNT];
};

struct tex {
	unsigned int scene;
	unsigned int off_lf[COUNT];
	unsigned int off_rt[COUNT];
};

unsigned int create_fbo_tex(int w, int h) {
	unsigned int tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	return tex;
}

int main(int argc, char *argv[]) {
	const float dt = 1 / 60.0;
	const float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	const v3f light_pos = _v3f(0.25, 0.25, 1);
	const v3f ambient = _v3f(0.05, 0.05, 0.05);
	const v3f specular = _v3f(0.5, 0.5, 0.5);
	const float shininess = 100;
	struct r3_ren ren;
	struct res res;
	m4f mv, mvp;
	struct fb fb;
	struct rb rb;
	struct tex tex;
	v2i size = { WINDOW_WIDTH, WINDOW_HEIGHT };
	float angle = 0;

	{ // Init
		r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
		ren.clear_color = _v3f(0.2, 0.2, 0.2);

		// on screen rb
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glGenRenderbuffers(1, &rb.on);
		glBindRenderbuffer(GL_RENDERBUFFER, rb.on);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, size.x, size.y);

		// on fb
		glGenFramebuffers(1, &fb.on);
		glBindFramebuffer(GL_FRAMEBUFFER, fb.on);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb.on);
		glBindRenderbuffer(GL_RENDERBUFFER, rb.on);

		// depth buf off fb
		glGenRenderbuffers(1, &rb.depth);
		glBindRenderbuffer(GL_RENDERBUFFER, rb.depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.x, size.y);

		glGenRenderbuffers(1, &rb.color);
		glBindRenderbuffer(GL_RENDERBUFFER, rb.color);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, size.x, size.y);

		glGenFramebuffers(1, &fb.scene);
		glBindFramebuffer(GL_FRAMEBUFFER, fb.scene);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb.color);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb.depth);
		tex.scene = create_fbo_tex(size.x, size.y);
		printf("w:%d, h:%d\n", size.x, size.y);

		for (int i = 0, w = size.x, h = size.y; i < COUNT; i++, w >>= 1, h >>= 1) {
			glGenRenderbuffers(1, &rb.off_lf[i]);
			glBindRenderbuffer(GL_RENDERBUFFER, rb.off_lf[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);

			glGenFramebuffers(1, &fb.off_lf[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, fb.off_lf[i]);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb.off_lf[i]);
			tex.off_lf[i] = create_fbo_tex(w, h);

			glGenRenderbuffers(1, &rb.off_rt[i]);
			glBindRenderbuffer(GL_RENDERBUFFER, rb.off_rt[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, w, h);

			glGenFramebuffers(1, &fb.off_rt[i]);
			glBindFramebuffer(GL_FRAMEBUFFER, fb.off_rt[i]);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb.off_rt[i]);
			tex.off_rt[i] = create_fbo_tex(w, h);
		}
	}
	{ // Setup cube
		r3_make_normal_shader(res.shader + RES_NORMAL);
		r3_make_color_shader(res.shader + RES_COLOR);
		r3_make_texture_shader(res.shader + RES_TEXTURE);
		struct r3_spec *spec = r3_create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &res.mesh);
		free(spec);
		res.tex_id = r3_load_tga_texture("res/img/base_map.tga");
		res.mode = RES_COLOR;
	}
	bool done = false;
	while (!done) { // Main loop
		{ // Input
			SDL_Event event;
			while (SDL_PollEvent(&event) && !done) {
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
				}
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) {
				res.mode++;
					res.mode %= RES_MODE_COUNT;
				}
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) {
					res.mode += RES_MODE_COUNT - 1;
					res.mode %= RES_MODE_COUNT;
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
			glBindFramebuffer(GL_FRAMEBUFFER, fb.on);
			r3_viewport(&ren);
			r3_clear(&ren);
			r3_enable_tests(&ren);
			switch (res.mode) {
			case RES_NORMAL:
				r3_render_normal(
					&res.mesh, res.shader + res.mode, res.tex_id, mv, mvp,
					light_pos, ambient, specular, shininess
				);
				break;
			case RES_COLOR:
				r3_render_color(&res.mesh, res.shader + res.mode, mvp);
				break;
			case RES_TEXTURE:
				r3_render_texture(&res.mesh, res.shader + res.mode, res.tex_id, mvp);
				break;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			r3_render(&ren);
		}
		SDL_Delay(16);
	}
	// Clean up
	r3_break_mesh(&res.mesh);
	for (int i = 0; i < RES_MODE_COUNT; i++) {
		r3_break_shader(res.shader + i);
	}
	r3_quit(&ren);
	return EXIT_SUCCESS;
}
