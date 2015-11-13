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
        const v3f kolor = _v3f(0.3, 0.9, 0.1);

        r3_normal_t normal = {
            .mv = eyem3f(),
            .light_position = _v3f(.25, .25, .25),
            .ambient_color = _v3f(.0, .0, .0),
            .diffuse_color = _v3f(1, 1, 1),
            .shininess = 100,
        };
        const unsigned int tex = r3_load_tga_texture("r3.tga");

        bool done = false;
        float angle = 0.f;
        int mode = 0;
        const int modes = 11;
        while (!done) {
                const int start_tick = SDL_GetTicks();

                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_KEYDOWN) {
                            switch (event.key.keysym.sym) {
                            case SDLK_ESCAPE: done = true; break;
                            case SDLK_LEFT: mode = (mode + modes - 1) % modes; break;
                            case SDLK_RIGHT: mode = (mode + 1) % modes; break;
                            default: break;
                            }
                        }
                }

                angle = fmodf(angle + dt * 3.5f, M_PI * 2);
                const m4f persp = perspf(45, aspect, 1, 20);
                const m4f translate = translatef(_v3f(0, 0, -7));
                const m4f rot = rotm4f(angle, _v3f(0.9, 0.6, 0.1));
                const m4f mv = addm4f(mulm4f(rot, scalem4f(eyem4f(), _v3f(3,3,3))), translate);
                const m4f mvp = mulm4f(persp, mv);
                normal.mv = m3m4f(mv);

                r3_viewport();
                r3_clear(_v3f(.2,.3,.3), R3_CLEAR_BIT_COLOR | R3_CLEAR_BIT_DEPTH);

                switch (mode) {
                case 0: r3_render_k(r3_cube_mesh(), mvp, kolor); break;
                case 1: r3_render_c(r3_cube_mesh(), mvp); break;
                case 2: r3_render_n(r3_cube_mesh(), mvp, &normal); break;
                case 3: r3_render_t(r3_cube_mesh(), mvp, tex); break;
                case 4: r3_render_kn(r3_cube_mesh(), mvp, kolor, &normal); break;
                case 5: r3_render_cn(r3_cube_mesh(), mvp, &normal); break;
                case 6: r3_render_kt(r3_cube_mesh(), mvp, kolor, tex); break;
                case 7: r3_render_ct(r3_cube_mesh(), mvp, tex); break;
                case 8: r3_render_nt(r3_cube_mesh(), mvp, &normal, tex); break;
                case 9: r3_render_knt(r3_cube_mesh(), mvp, kolor, &normal, tex); break;
                case 10: r3_render_cnt(r3_cube_mesh(), mvp, &normal, tex); break;
                default: break;
                }
                r3_render();

                const int end_tick = SDL_GetTicks();
                const int diff_tick = end_tick - start_tick;
                if (diff_tick < 16)
                        SDL_Delay(16 - diff_tick);
        }
        r3_quit();
        return 0;
}
