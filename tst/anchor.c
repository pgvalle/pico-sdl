#include "../src/pico.h"

int main (void) {
    pico_init(1);
    pico_set_title("Anchoring...");
    pico_set_size_internal((SDL_Point){10,10});
    pico_set_size_external((SDL_Point){200,200});

    // PIXELS
    SDL_Point pt = pico_pct_to_pos(0.5, 0.5);
    {
        pico_set_anchor(PICO_CENTER, PICO_MIDDLE);
        pico_output_draw_pixel(pt);
        pico_output_present();
        puts("pixel centralizado - center/middle - fica 1px pra dir/baixo");
        pico_input_event(NULL, SDL_KEYDOWN);

        pico_set_anchor(PICO_LEFT, PICO_TOP);
        pico_output_draw_pixel(pt);
        pico_output_present();
        puts("pixel centralizado - left/top - fica 1px pra dir/baixo");
        pico_input_event(NULL, SDL_KEYDOWN);

        pico_set_anchor(PICO_RIGHT, PICO_BOTTOM);
        pico_output_draw_pixel(pt);
        pico_output_present();
        puts("pixel centralizado - right/bottom - fica 1px pra esq/cima");
        pico_input_event(NULL, SDL_KEYDOWN);
    }

    // RECTS
    SDL_Rect rct = { pt.x, pt.y, 4, 4 };
    {
        pico_set_anchor(PICO_CENTER, PICO_MIDDLE);
        pico_output_draw_rect(rct);
        pico_output_present();
        puts("retangulo centralizado - fica exato");
        pico_input_event(NULL, SDL_KEYDOWN);

        pico_set_anchor(PICO_LEFT, PICO_TOP);
        pico_output_draw_rect(rct);
        pico_output_present();
        puts("retangulo left/top - sobra uma lin/col");
        pico_input_event(NULL, SDL_KEYDOWN);

        pico_set_anchor(PICO_RIGHT, PICO_BOTTOM);
        pico_output_draw_rect(rct);
        pico_output_present();
        puts("retangulo right/bottom - sobra uma lin/col");
        pico_input_event(NULL, SDL_KEYDOWN);

        pico_set_anchor(PICO_RIGHT, PICO_MIDDLE);
        pico_output_draw_rect(rct);
        pico_output_present();
        puts("retangulo right/middle - sobra uma col");
        pico_input_event(NULL, SDL_KEYDOWN);
    }

    pico_init(0);
    return 0;
}
