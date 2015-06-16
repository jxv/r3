#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ml.h>
#include <r3_sdl.h>
#include <GLES2/gl2.h>

#ifdef GCW0
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240
#define COUNT 3
#else
#define WINDOW_WIDTH (320)
#define WINDOW_HEIGHT (240)
#define COUNT 3
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

int main(int argc, char *argv[])
{
	{ // Init
		r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
		// default screen fb
		on.fbo = 0;
		glBindFramebuffer(GL_FRAMEBUFFER, on.fbo);
		on.size = _v2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		// rb that will serve as the color attachment for the framebuffer.
		glGenRenderbuffers(1, &on.color);
		glBindRenderbuffer(GL_RENDERBUFFER, on.color);
		// glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, on.size.x, on.size.y);
		// Build the framebuffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, on.color);
		glBindRenderbuffer(GL_RENDERBUFFER, on.color);
		on.tex = r3_make_fbo_tex(on.size.x, on.size.y);
		// check completion
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return EXIT_FAILURE;
		}

		// fb for off-screen
		glGenFramebuffers(1, &off.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, off.fbo);
		off.size = _v2i(WINDOW_WIDTH, WINDOW_HEIGHT);
		// rb for depth and color attachment for fb
		glGenRenderbuffers(1, &off.color);
		glBindRenderbuffer(GL_RENDERBUFFER, off.color);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, off.size.x, off.size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, off.color);
		glGenRenderbuffers(1, &off.depth);
		glBindRenderbuffer(GL_RENDERBUFFER, off.depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, off.size.x, off.size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off.depth);
		off.tex = r3_make_fbo_tex(off.size.x, off.size.y);
		// check completion
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			return EXIT_FAILURE;
		}

		for (int w = on.size.x, h = on.size.y, i = 0; i < COUNT; i++, w >>= 1, h >>= 1) {
			glGenFramebuffers(1, &lf[i].fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
			lf[i].size = _v2i(w,h);
			glGenRenderbuffers(1, &lf[i].color);
			glBindRenderbuffer(GL_RENDERBUFFER, lf[i].color);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, lf[i].color);
			lf[i].tex = r3_make_fbo_tex(w,h);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				return EXIT_FAILURE;
			}

			
			glGenFramebuffers(1, &rt[i].fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
			rt[i].size = _v2i(w,h);
			glGenRenderbuffers(1, &rt[i].color);
			glBindRenderbuffer(GL_RENDERBUFFER, rt[i].color);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rt[i].color);
			rt[i].tex = r3_make_fbo_tex(w,h);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				return EXIT_FAILURE;
			}
		}
		
		//r3_make_normal_shader(&res.shader);
		r3_make_color_normal_texture_shader(&res.shader);
		struct r3_spec *spec = r3_create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &res.mesh);
		free(spec);
		res.tex = r3_load_tga_texture("res/img/base_map.tga");
		
		quad = r3_make_quad();

		r3_make_blit_shader(&blit);
		r3_make_blit_alpha_shader(&blit_alpha);
		r3_make_blur_shader(&blur);
		r3_make_high_pass_shader(&high_pass);
		glUseProgram(high_pass.program);
		glUniform1f(high_pass.uniform.threshold, 0.0);

		r3_enable_tests(&ren);
	}
	bool done = false;
	while (!done) { // Main loop
		const int start_tick = SDL_GetTicks();
		{ // Input
			SDL_Event event;
			while (SDL_PollEvent(&event) && !done) {
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
					done = true;
				}
			}
		}
		{ // Update
			angle = fmodf(angle + dt * 3.5, M_PI * 2);
			const m4f persp = perspf(45, aspect, 1, 20);
			const m4f translate = translatef(_v3f(cosf(angle*2)*1.8,sinf(angle*5),-7));
			const m4f rot = rotm4f(angle, _v3f(0.9, 0.5, 0.1));
			mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3, 3, 3))), translate);
			mvp = mulm4f(persp, mv);
		}
		{ // Render
			// off screen
			glBindFramebuffer(GL_FRAMEBUFFER, off.fbo);
			glViewport(0, 0, off.size.x, off.size.y);
			glClearColor(0,0,0,1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//glDepthFunc(GL_ALWAYS);
			//render_blit(&quad, res.tex);
			//glDepthFunc(GL_LESS);
			r3_render_color_normal_texture(&res.mesh, &res.shader, res.tex, mv, mvp, light_pos, ambient, specular, shininess);

			glDepthFunc(GL_ALWAYS);
			for (int i = 0; i < COUNT; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				r3_render_blit(&quad, &blit, i == 0 ? off.tex : rt[i - 1].tex);

				glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
				glViewport(0, 0, rt[i].size.x, rt[i].size.y);
				r3_render_high_pass(&quad, &high_pass, lf[i].tex);

				glBindFramebuffer(GL_FRAMEBUFFER, lf[i].fbo);
				glViewport(0, 0, lf[i].size.x, lf[i].size.y);
				r3_render_blur_width(&quad, &blur, rt[i].tex, WINDOW_WIDTH / (float)WINDOW_HEIGHT, WINDOW_WIDTH);

				glBindFramebuffer(GL_FRAMEBUFFER, rt[i].fbo);
				glViewport(0, 0, rt[i].size.x, rt[i].size.y);
				r3_render_blur_height(&quad, &blur, lf[i].tex, WINDOW_WIDTH / (float)WINDOW_HEIGHT, WINDOW_HEIGHT);
			}
			glDepthFunc(GL_LESS);
			
			glBindFramebuffer(GL_FRAMEBUFFER, on.fbo);
			glViewport(0, 0, on.size.x, on.size.y);
			glClearColor(0.2,0.2,0.2,1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//glDepthFunc(GL_ALWAYS);
			//render_blit(&quad, res.tex);
			//glDepthFunc(GL_LESS);

			glBlendFunc(GL_ONE, GL_ONE);
			glEnable(GL_BLEND);
			glDepthFunc(GL_ALWAYS);
			for (int i = 0; i < COUNT; i++) {
				r3_render_blit_alpha(&quad, &blit, rt[i].tex, 1);
			}
			glDepthFunc(GL_LESS);
			glDisable(GL_BLEND);
			
			r3_render_color_normal_texture(&res.mesh, &res.shader, res.tex, mv, mvp, light_pos, ambient, specular, shininess);
			
			r3_render(&ren);
		}
		const int end_tick = SDL_GetTicks();
		const int diff_tick = end_tick - start_tick;
		if (diff_tick < 24) {
			SDL_Delay(24 - diff_tick);
		}
	}
	// Clean up
	r3_break_mesh(&res.mesh);
	r3_break_shader(&res.shader);
	r3_quit(&ren);
	return EXIT_SUCCESS;
}

