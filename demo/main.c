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

int main(int argc, char *argv[]) {
	const float dt = 1 / 60.0;
	const float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	struct r3_ren ren;
	struct r3_resource res;
	m4f mv, mvp;
	float angle = 0;
	// Init
	r3_sdl_init("", _v2i(WINDOW_WIDTH, WINDOW_HEIGHT), &ren);
	r3_viewport(&ren);
	r3_enable_tests(&ren);
	ren.clear_color = _v3f(0.2, 0.2, 0.2);
	// Setup cube
	r3_make_normal_shader(&res.shader);
	struct r3_spec *spec = r3_create_cuboid_spec();
	r3_make_mesh_from_spec(spec, &res.mesh);
	free(spec);
	res.tex_id[0] = r3_load_tga_texture("res/img/base_map.tga");
	// Main loop
	bool done = false;
	while (!done) {
		// Input
		SDL_Event event;
		while (SDL_PollEvent(&event) && !done) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				done = true;
			}
		}
		// Update
		angle = fmodf(angle + dt * 2, M_PI * 2);
		const m4f persp = perspm4f(45, aspect, 1, 20);
		const m4f translate = translatem4f(_v3f(0,0,-7));
		const m4f rot = rotm4f(angle, _v3f(0.9,0.5,0.2));
		mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
		mvp = mulm4f(persp, mv);
		// Render
		r3_viewport(&ren);
		r3_clear(&ren);
		r3_render_resource(&res, mv, mvp, _v3f(0.25, 0.25, 1), _v3f(0.05, 0.05, 0.05), _v3f(0.5, 0.5, 0.5), 100);
		r3_render(&ren);
		//
		SDL_Delay(16);
	}
	// Clean up
	r3_break_mesh(&res.mesh);
	r3_break_shader(&res.shader);
	r3_quit(&ren);
	return EXIT_SUCCESS;
}
