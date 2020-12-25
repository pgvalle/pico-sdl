#include <assert.h>
#include <SDL2/SDL.h>

#define TITLE "pico-SDL"
#define WIN_DIM 510
#define pico_assert(x) if (!(x)) { fprintf(stderr,"%s\n",SDL_GetError()); assert(0 && "SDL ERROR"); }

typedef struct {
    int v1, v2;
} Pico_2i;

typedef struct {
    int v1, v2, v3, v4;
} Pico_4i;

typedef enum {
    DELAY
} INPUT;

typedef enum {
    SET,
    CLEAR,
    DRAW,
    UPDATE
} OUTPUT;

typedef enum {
    SIZE,
    COLOR_BG,
    COLOR_FG
} OUTPUT_SET;

typedef enum {
    PIXEL
} OUTPUT_DRAW;

typedef struct {
    INPUT sub;
    union {
        int Delay;     // INPUT_DELAY
    };
} Input;

typedef struct {
    OUTPUT sub;
    union {
        struct {
            OUTPUT_SET sub;
            union {
                struct {
                    int win_w, win_h, log_w, log_h;
                } Size;
                Pico_4i Color_BG;
                Pico_4i Color_FG;
            };
        } Set;
        struct {
            OUTPUT_DRAW sub;
            union {
                Pico_2i Pixel;
            };
        } Draw;
    };
} Output;

void init   ();
void input  (Input inp);
void output (Output out);
