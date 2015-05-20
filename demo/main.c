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
	RES_NORMAL,
	RES_COLOR,
	RES_TEXTURE,
};

int main(int argc, char *argv[]) {
	const float dt = 1 / 60.0;
	const float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	struct r3_ren ren;
	struct r3_resource res[3];
	enum res_mode mode;
	m4f mv, mvp;
	float angle = 0;
	{ // Init
		r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
		r3_viewport(&ren);
		r3_enable_tests(&ren);
		ren.clear_color = _v3f(0.2, 0.2, 0.2);
	}
	{ // Setup cube
		r3_make_normal_shader(&res[0].shader);
		r3_make_color_shader(&res[1].shader);
		r3_make_texture_shader(&res[2].shader);
		struct r3_spec *spec = r3_create_cuboid_spec();
		r3_make_mesh_from_spec(spec, &res[0].mesh);
		res[2].mesh = res[1].mesh = res[0].mesh;
		free(spec);
		res[0].tex_id = r3_load_tga_texture("res/img/base_map.tga");
		res[2].tex_id = res[1].tex_id = res[0].tex_id;
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
					mode++;
					mode %= 3;
				}
				if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) {
					mode += 3 - 1;
					mode %= 3;
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
			switch (mode) {
			case RES_NORMAL:
				r3_render_resource(&res[0], mv, mvp, _v3f(0.25, 0.25, 1), _v3f(0.05, 0.05, 0.05), _v3f(0.5, 0.5, 0.5), 100);
				break;
			case RES_COLOR:
				r3_render_resource_color(&res[1], mvp);
				break;
			case RES_TEXTURE:
				r3_render_resource_texture(&res[2], mvp);
				break;
			}
			r3_render(&ren);
		}
		SDL_Delay(16);
	}
	// Clean up
	r3_break_mesh(&res[0].mesh);
	r3_break_shader(&res[0].shader);
	r3_break_shader(&res[1].shader);
	r3_break_shader(&res[2].shader);
	r3_quit(&ren);
	return EXIT_SUCCESS;
}
