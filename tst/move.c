#include "../src/pico.h"

int main (void) {
    pico_init(1);
    pico_state_set_title("Moving Around");

    int x=0, y=0;
    while (1) {
        pico_state_set_color_draw((SDL_Color){0xFF,0xFF,0xFF,0xFF});
        pico_output_draw_pixel((SDL_Point){x,y});

        pico_output_present();
        pico_input_delay(200);

        y = y+1;
    }

    pico_init(0);
}
