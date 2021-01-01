#include "../pico.h"

int main (void) {
    pico_init();

    SDL_Event e1;
    int ok = pico_input((Pico_Input){ PICO_EVENT, .Event={SDL_KEYUP,&e1} });
    assert(ok);

    // TITLE
    pico_output((Pico_Output) { PICO_SET, .Set={PICO_TITLE,.Title="Hello World!"} });

    // CLEAR
    pico_output((Pico_Output) { PICO_SET, .Set={PICO_COLOR,.Color={PICO_COLOR_CLEAR,.Clear={0xFF,0xFF,0xFF,0x00}}} });
    pico_output((Pico_Output) { PICO_CLEAR });
    pico_input((Pico_Input){ PICO_DELAY, .Delay=2000 });

    // DRAW_PIXEL
    pico_output((Pico_Output) { PICO_SET, .Set={PICO_COLOR,.Color={PICO_COLOR_CLEAR,.Clear={0x00,0x00,0x00,0x00}}} });
    pico_output((Pico_Output) { PICO_SET, .Set={PICO_COLOR,.Color={PICO_COLOR_DRAW,.Draw={0xFF,0xFF,0xFF,0x00}}} });
    pico_output((Pico_Output) { PICO_CLEAR });
    pico_output((Pico_Output) { PICO_DRAW, .Draw={PICO_PIXEL,.Pixel={0,0}} });
    pico_input((Pico_Input){ PICO_DELAY, .Delay=2000 });

    // DRAW_TEXT
    pico_output((Pico_Output) { PICO_DRAW, .Draw={PICO_TEXT,.Text={{0,0},"Hello!"}} });
    pico_input((Pico_Input){ PICO_DELAY, .Delay=2000 });

    // EVENT
    SDL_Event e2;
    pico_input((Pico_Input){ PICO_EVENT_TIMEOUT, .Event_Timeout={SDL_ANY,5000,&e2} });

    return 0;
}
