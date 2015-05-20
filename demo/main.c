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
	float angle = 0;
	{ // Init
		r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
		r3_viewport(&ren);
		r3_enable_tests(&ren);
		ren.clear_color = _v3f(0.2, 0.2, 0.2);
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
			const m4f persp = perspm4f(45, aspect, 1, 20);
			const m4f translate = translatem4f(_v3f(0,0,-7));
			const m4f rot = rotm4f(angle, _v3f(0.9,0.5,0.2));
			mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
			mvp = mulm4f(persp, mv);
		}
		{ // Render
			r3_viewport(&ren);
			r3_clear(&ren);
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
