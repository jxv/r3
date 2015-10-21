#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ml.h>
#include "../../include/r3.h"
#include <SDL2/SDL.h>
#include <GLES2/gl2.h>

int main()
{
        r3_init("", _v2i(320, 240));
        const float dt = 1.f / 60.f;
        const float aspect = 320.f / 240.f;

        r3_normal_t normal = {
            .light_position = _v3f(.25, .25, .25),
            .ambient_color = _v3f(.0, .0, .0),
            .diffuse_color = _v3f(1, 1, 1),
            .shininess = 100,
        };
        const unsigned int tex = r3_load_tga_texture("r3.tga");

        bool done = false;
        float angle = 0.f;
        while (!done) {
                const int start_tick = SDL_GetTicks();

                SDL_Event event;
                while (SDL_PollEvent(&event))
                        done |= event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE;

                angle = fmodf(angle + dt * 3.5f, M_PI * 2);
                const m4f persp = perspf(45, aspect, 1, 20);
                const m4f translate = translatef(_v3f(0, 0, -7));
                const m4f rot = rotm4f(angle, _v3f(0.9, 0.6, 0.1));
                const m4f mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
                const m4f mvp = mulm4f(persp, mv);
                normal.mv = m3m4f(mv);

                r3_viewport();
                r3_clear(_v3f(.2,.3,.3), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                //r3_render_pc(r3_cube_mesh(), mvp);
                //r3_render_pn(r3_cube_mesh(), mvp, &normal);
                //r3_render_pt(r3_cube_mesh(), mvp, tex);
                //r3_render_pcn(r3_cube_mesh(), mvp, &normal);
                //r3_render_pct(r3_cube_mesh(), mvp, tex);
                r3_render_pnt(r3_cube_mesh(), mvp, &normal, tex);
                //r3_render_pcnt(r3_cube_mesh(), mvp, &normal, tex);
                r3_render();

                const int end_tick = SDL_GetTicks();
                const int diff_tick = end_tick - start_tick;
                if (diff_tick < 16)
                        SDL_Delay(16 - diff_tick);
        }
        r3_quit();
        return 0;
}
