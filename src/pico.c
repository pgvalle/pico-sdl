#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "hash.h"
#include "pico.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

static SDL_Window*  WIN;
static SDL_Texture* TEX;

#define REN (SDL_GetRenderer(WIN))

#define X(v,w) (hanchor(v,w)-S.pan.x)
#define Y(v,h) (vanchor(v,h)-S.pan.y)

#define LOG ({Pico_Dim log; SDL_RenderGetLogicalSize(REN, &log.x, &log.y); log;})
#define PHY ({Pico_Dim phy; SDL_GetWindowSize(WIN, &phy.x, &phy.y); phy;})

static pico_hash* _pico_hash;

typedef SDL_Point Pico_Anchor;

static struct {
    Pico_Anchor anchor;
    struct {
        Pico_Color clear;
        Pico_Color draw;
    } color;
    struct {
        int x;
        Pico_Pos cur;
    } cursor;
    int expert;
    struct {
        TTF_Font* ttf;
        int h;
    } font;
    int grid;
    struct {
        Pico_Rect crop;
        Pico_Dim  size;
    } image;
    Pico_Pos pan;
    Pico_Style style;
} S = {
    { PICO_CENTER, PICO_MIDDLE },
    { {0x00,0x00,0x00,0xFF}, {0xFF,0xFF,0xFF,0xFF} },
    {0, {0,0}},
    0,
    {NULL, 0},
    1,
    { {0,0,0,0}, {0,0} },
    {0, 0},
    PICO_FILL
};

static int hanchor (int x, int w) {
    switch (S.anchor.x) {
        case PICO_LEFT:
            return x;
        case PICO_CENTER:
            return x - w/2;
        case PICO_RIGHT:
            return x - w; // + 1;
    }
    assert(0 && "bug found");
}

static int vanchor (int y, int h) {
    switch (S.anchor.y) {
        case PICO_TOP:
            return y;
        case PICO_MIDDLE:
            return y - h/2;
        case PICO_BOTTOM:
            return y - h; // + 1;
    }
    assert(0 && "bug found");
}

// UTILS

int pico_is_point_in_rect (Pico_Pos pt, Pico_Rect r) {
    int rw = r.w / 2;
    int rh = r.h / 2;
    return !(pt.x<r.x-rw || pt.x>r.x+rw || pt.y<r.y-rh || pt.y>r.y+rh);
}

Pico_Pos pico_pct_to_pos (int x, int y) {
    Pico_Dim log = LOG;
    return pico_pct_to_pos_ext((Pico_Rect){log.x/2,log.y/2,log.x,log.y}, x, y);
}

Pico_Pos pico_pct_to_pos_ext (Pico_Rect r, int x, int y) {
    return (Pico_Pos) { r.x-r.w/2 + r.w*x/100, r.y-r.h/2 + r.h*y/100 };
}

// INIT

void pico_init (int on) {
    if (on) {
        _pico_hash = pico_hash_create(PICO_HASH);
        pico_assert(0 == SDL_Init(SDL_INIT_VIDEO));
        WIN = SDL_CreateWindow (
            PICO_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            PICO_DIM_PHY.x, PICO_DIM_PHY.y, SDL_WINDOW_SHOWN
        );
        pico_assert(WIN != NULL);

        SDL_CreateRenderer(WIN, -1, SDL_RENDERER_ACCELERATED);
        //SDL_CreateRenderer(WIN, -1, SDL_RENDERER_SOFTWARE);
        pico_assert(REN != NULL);
        SDL_SetRenderDrawBlendMode(REN, SDL_BLENDMODE_BLEND);

        TTF_Init();
        Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 4096);

        pico_set_size(PICO_DIM_PHY, PICO_DIM_LOG);
        pico_set_font(NULL, 0);
    } else {
        if (S.font.ttf != NULL) {
            TTF_CloseFont(S.font.ttf);
        }
        Mix_CloseAudio();
        TTF_Quit();
        SDL_DestroyRenderer(REN);
        SDL_DestroyWindow(WIN);
        SDL_Quit();
        pico_hash_destroy(_pico_hash);
    }
}

// INPUT

// Pre-handles input from environment:
//  - SDL_QUIT: quit
//  - CTRL_-/=: pixel
//  - CTRL_L/R/U/D: pan
//  - receives:
//      - e:  actual input
//      - xp: input I was expecting
//  - returns
//      - 1: if e matches xp
//      - 0: otherwise
static int event_from_sdl (SDL_Event* e, int xp) {
    switch (e->type) {
        case SDL_QUIT: {
            if (!S.expert) {
                exit(0);
            }
            break;
        }

        case SDL_KEYDOWN: {
            const unsigned char* state = SDL_GetKeyboardState(NULL);
            if (!state[SDL_SCANCODE_LCTRL] && !state[SDL_SCANCODE_RCTRL]) {
                break;
            }
            switch (e->key.keysym.sym) {
                case SDLK_0: {
                    pico_set_size(PICO_DIM_PHY, PICO_DIM_LOG);
                    pico_set_pan((Pico_Pos){0, 0});
                    break;
                }
                case SDLK_MINUS: {
                    Pico_Dim log = LOG;
                    log.x *= 1.1;
                    log.y *= 1.1;
                    pico_set_size(PICO_SIZE_KEEP, log);
                    break;
                }
                case SDLK_EQUALS: {
                    Pico_Dim log = LOG;
                    log.x *= 0.9;
                    log.y *= 0.9;
                    pico_set_size(PICO_SIZE_KEEP, log);
                    break;
                }
                case SDLK_LEFT: {
                    pico_set_pan((Pico_Pos){S.pan.x-5, S.pan.y});
                    break;
                }
                case SDLK_RIGHT: {
                    pico_set_pan((Pico_Pos){S.pan.x+5, S.pan.y});
                    break;
                }
                case SDLK_UP: {
                    pico_set_pan((Pico_Pos){S.pan.x, S.pan.y-5});
                    break;
                }
                case SDLK_DOWN: {
                    pico_set_pan((Pico_Pos){S.pan.x, S.pan.y+5});
                    break;
                }
            }
        }

#if 0
        case SDL_WINDOWEVENT: {
            if (e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                pico_set_size((SDL_Point){e->window.data1,e->window.data2});
                return 0;
                break;
            }
        }
#endif
        default:
            // others are not handled automatically
            break;
    }

    if (xp == (int)e->type) {
        // OK
    } else if (xp == SDL_ANY) {
        // MAYBE
        if (e->type==SDL_KEYDOWN || e->type==SDL_KEYUP || e->type==SDL_MOUSEBUTTONDOWN ||
            e->type==SDL_MOUSEBUTTONUP || e->type==SDL_MOUSEMOTION ||
            e->type==SDL_QUIT) {
            // OK
        } else {
            // NO
            return 0;   // not the one I was expecting
        }
    } else {
        // NO
        return 0;   // not the one I was expecting
    }

    // adjusts SDL -> logical positions
    switch (e->type) {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
            e->button.x = e->button.x + S.pan.x;
            e->button.y = e->button.y + S.pan.y;
            break;
        default:
            break;
    }
    return 1;
}

void pico_input_delay (int ms) {
    while (1) {
        int old = SDL_GetTicks();
        SDL_Event e;
        int has = SDL_WaitEventTimeout(&e, ms);
        if (has) {
            event_from_sdl(&e, SDL_ANY);
        }
        int dt = SDL_GetTicks() - old;
        ms -= dt;
        if (ms <= 0) {
            return;
        }
    }
}

void pico_input_event (SDL_Event* evt, int type) {
    while (1) {
        SDL_Event x;
        SDL_WaitEvent(&x);
        if (event_from_sdl(&x, type)) {
            if (evt != NULL) {
                *evt = x;
            }
            return;
        }
    }
}

int pico_input_event_ask (SDL_Event* evt, int type) {
    int has = SDL_PollEvent(evt);
    if (!has) return 0;
    return event_from_sdl(evt, type);
}

int pico_input_event_timeout (SDL_Event* evt, int type, int timeout) {
    int has = SDL_WaitEventTimeout(evt, timeout);
    if (!has) {
        return 0;
    }
    if (event_from_sdl(evt, type)) {
        return 1;
    }
    return 0;
}

// OUTPUT

static void _pico_output_clear (void) {
    SDL_SetRenderDrawColor (REN,
        S.color.clear.r,
        S.color.clear.g,
        S.color.clear.b,
        S.color.clear.a
    );
    SDL_RenderClear(REN);
    SDL_SetRenderDrawColor (REN,
        S.color.draw.r,
        S.color.draw.g,
        S.color.draw.b,
        S.color.draw.a
    );
}

static void _pico_output_present (int force);
void pico_output_clear (void) {
    _pico_output_clear();
    _pico_output_present(0);
}

static void _pico_output_draw_image_tex (Pico_Pos pos, SDL_Texture* tex) {
    Pico_Rect rct;
    SDL_QueryTexture(tex, NULL, NULL, &rct.w, &rct.h);

    Pico_Rect crp = S.image.crop;
    if (S.image.crop.w == 0) {
        crp.w = rct.w;
    }
    if (S.image.crop.h == 0) {
        crp.h = rct.h;
    }

    if (S.image.size.x==0 && S.image.size.y==0) {
        // normal image size
        rct.w = crp.w;  // (or copy from crop)
        rct.h = crp.h;  // (or copy from crop)
    } else if (S.image.size.x == 0) {
        // adjust w based on h
        rct.w = rct.w * (S.image.size.y / (float)rct.h);
        rct.h = S.image.size.y;
    } else if (S.image.size.y == 0) {
        // adjust h based on w
        rct.h = rct.h * (S.image.size.x / (float)rct.w);
        rct.w = S.image.size.x;
    } else {
        rct.w = S.image.size.x;
        rct.h = S.image.size.y;
    }

    // SCALE
    rct.w = rct.w; // * GRAPHICS_SET_SCALE_W;
    rct.h = rct.h; // * GRAPHICS_SET_SCALE_H;

    // ANCHOR / PAN
    rct.x = X(pos.x, rct.w);
    rct.y = Y(pos.y, rct.h);

    SDL_RenderCopy(REN, tex, &crp, &rct);
    _pico_output_present(0);
}

static void _pico_output_draw_image_cache (Pico_Pos pos, const char* path, int cache) {
    SDL_Texture* tex = NULL;
    if (cache) {
        tex = pico_hash_get(_pico_hash, path);
        if (tex == NULL) {
            tex = IMG_LoadTexture(REN, path);
            pico_hash_add(_pico_hash, path, tex);
        }
    } else {
        tex = IMG_LoadTexture(REN, path);
    }
    pico_assert(tex != NULL);

    _pico_output_draw_image_tex(pos, tex);

    if (!cache) {
        SDL_DestroyTexture(tex);
    }
}

void pico_output_draw_image (Pico_Pos pos, const char* path) {
    _pico_output_draw_image_cache(pos, path, 1);
}

void pico_output_draw_line (Pico_Pos p1, Pico_Pos p2) {
    SDL_RenderDrawLine(REN, X(p1.x,1),Y(p1.y,1), X(p2.x,1),Y(p2.y,1));
    _pico_output_present(0);
}

void pico_output_draw_pixel (Pico_Pos pos) {
    SDL_RenderDrawPoint(REN, X(pos.x,1), Y(pos.y,1) );
    _pico_output_present(0);
}

void pico_output_draw_pixels (const Pico_Pos* poss, int count) {
    Pico_Pos vec[count];
    for (int i=0; i<count; i++) {
        vec[i].x = X(poss[i].x,1);
        vec[i].y = Y(poss[i].y,1);
    }
    SDL_RenderDrawPoints(REN, vec, count);
    _pico_output_present(0);
}

void pico_output_draw_rect (Pico_Rect rect) {
    Pico_Rect out = {
        X(rect.x, rect.w),
        Y(rect.y, rect.h),
        rect.w, rect.h
    };
    switch (S.style) {
        case PICO_FILL:
            SDL_RenderFillRect(REN, &out);
            break;
        case PICO_STROKE:
            SDL_RenderDrawRect(REN, &out);
            break;
    }
    _pico_output_present(0);
}

void pico_output_draw_oval (Pico_Rect rect) {
    Pico_Rect out = {
        X(rect.x, rect.w),
        Y(rect.y, rect.h),
        rect.w, rect.h
    };
    switch (S.style) {
        case PICO_FILL:
            filledEllipseRGBA (
                REN,
                out.x+out.w/2, out.y+out.h/2, out.w/2, out.h/2,
                S.color.draw.r, S.color.draw.g, S.color.draw.b, S.color.draw.a
            );
            break;
        case PICO_STROKE:
            ellipseRGBA (
                REN,
                out.x+out.w/2, out.y+out.h/2, out.w/2, out.h/2,
                S.color.draw.r, S.color.draw.g, S.color.draw.b, S.color.draw.a
            );
            break;
    }
    _pico_output_present(0);
}

void pico_output_draw_text (Pico_Pos pos, const char* text) {
    uint8_t r, g, b, a;
    SDL_GetRenderDrawColor(REN, &r,&g,&b,&a);
    pico_assert(S.font.ttf != NULL);
    SDL_Surface* sfc = TTF_RenderText_Blended(S.font.ttf, text,
                                              (Pico_Color){r,g,b,a});
    pico_assert(sfc != NULL);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(REN, sfc);
    pico_assert(tex != NULL);

    Pico_Rect rct;

    // SCALE
    rct.w = sfc->w; // * GRAPHICS_SET_SCALE_W;
    rct.h = sfc->h; // * GRAPHICS_SET_SCALE_H;

    // ANCHOR
    rct.x = X(pos.x, rct.w);
    rct.y = Y(pos.y, rct.h);

    SDL_RenderCopy(REN, tex, NULL, &rct);
    _pico_output_present(0);

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(sfc);
}

static void show_grid (void) {
    if (!S.grid) return;

    SDL_SetRenderDrawColor(REN, 0x77,0x77,0x77,0x77);

    SDL_SetRenderTarget(REN, TEX);      // HACK-01: LOG fails w/ this line
    Pico_Dim phy = PHY;
    Pico_Dim log = LOG;
    SDL_SetRenderTarget(REN, NULL);     // HACK-01

    SDL_RenderSetLogicalSize(REN, phy.x, phy.y);
    for (int i=0; i<=phy.x; i+=(phy.x/log.x)) {
        SDL_RenderDrawLine(REN, i, 0, i, phy.y);
    }
    for (int j=0; j<=phy.y; j+=(phy.y/log.y)) {
        SDL_RenderDrawLine(REN, 0, j, phy.x, j);
    }
    SDL_RenderSetLogicalSize(REN, log.x, log.y);

    SDL_SetRenderDrawColor (REN,
        S.color.draw.r,
        S.color.draw.g,
        S.color.draw.b,
        S.color.draw.a
    );
}

static void _pico_output_present (int force) {
    if (S.expert && !force) return;
    SDL_SetRenderTarget(REN, NULL);
    SDL_RenderClear(REN);
    SDL_RenderCopy(REN, TEX, NULL, NULL);
    show_grid();
    SDL_RenderPresent(REN);
    SDL_SetRenderTarget(REN, TEX);
}

void pico_output_present (void) {
    _pico_output_present(1);
}

static void _pico_output_sound_cache (const char* path, int cache) {
    Mix_Chunk* mix = NULL;

    if (cache) {
        mix = pico_hash_get(_pico_hash, path);
        if (mix == NULL) {
            mix = Mix_LoadWAV(path);
            pico_hash_add(_pico_hash, path, mix);
        }
    } else {
        mix = Mix_LoadWAV(path);
    }
    pico_assert(mix != NULL);

    Mix_PlayChannel(-1, mix, 0);

    if (!cache) {
        Mix_FreeChunk(mix);
    }
}

void pico_output_sound (const char* path) {
    _pico_output_sound_cache(path, 1);
}

static void _pico_output_write_aux (const char* text, int isln) {
    if (strlen(text) == 0) {
        if (isln) {
            S.cursor.cur.x = S.cursor.x;
            S.cursor.cur.y += S.font.h;
        }
        return;
    }

    pico_assert(S.font.ttf != NULL);
    SDL_Surface* sfc = TTF_RenderText_Blended (
        S.font.ttf, text,
        (Pico_Color) { S.color.draw.r, S.color.draw.g,
                       S.color.draw.b, S.color.draw.a }
    );
    pico_assert(sfc != NULL);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(REN, sfc);
    pico_assert(tex != NULL);

    int w, h;
    TTF_SizeText(S.font.ttf, text, &w,&h);
    Pico_Rect rct = { X(S.cursor.cur.x,0),Y(S.cursor.cur.y,0), w,h };
    SDL_RenderCopy(REN, tex, NULL, &rct);
    _pico_output_present(0);

    S.cursor.cur.x += w;
    if (isln) {
        S.cursor.cur.x = S.cursor.x;
        S.cursor.cur.y += S.font.h;
    }

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(sfc);
}

void pico_output_write (const char* text) {
    _pico_output_write_aux(text, 0);
}

void pico_output_writeln (const char* text) {
    _pico_output_write_aux(text, 1);
}


// STATE

// GET

Pico_Dim pico_get_image_size (const char* file) {
    SDL_Texture* tex = IMG_LoadTexture(REN, file);
    pico_assert(tex != NULL);
    Pico_Dim size;
    SDL_QueryTexture(tex, NULL, NULL, &size.x, &size.y);
    return size;
}

Pico_Size pico_get_size (void) {
    return (Pico_Size) { PHY, LOG };
}

Uint32 pico_get_ticks (void) {
    return SDL_GetTicks();
}

// SET

void pico_set_anchor (Pico_Anchor_X x, Pico_Anchor_Y y) {
    S.anchor = (Pico_Anchor) {x, y};
}

void pico_set_color_clear (Pico_Color color) {
    S.color.clear = color;
}

void pico_set_color_draw  (Pico_Color color) {
    S.color.draw = color;
    SDL_SetRenderDrawColor (REN,
        S.color.draw.r,
        S.color.draw.g,
        S.color.draw.b,
        S.color.draw.a
    );
}

void pico_set_cursor (Pico_Pos pos) {
    S.cursor.cur = pos;
    S.cursor.x   = pos.x;
}

void pico_set_expert (int on) {
    S.expert = on;
}

void pico_set_font (const char* file, int h) {
    if (file == NULL) {
        static char _file[255];
        strcpy(_file, __FILE__);
        _file[strlen(_file) - strlen("pico.c") - 1] = '\0';
        strcat(_file, "/../tiny.ttf");
        file = _file;
    }
    if (h == 0) {
        h = MAX(8, LOG.y/10);
    }
    S.font.h = h;
    if (S.font.ttf != NULL) {
        TTF_CloseFont(S.font.ttf);
    }
    S.font.ttf = TTF_OpenFont(file, S.font.h);
    pico_assert(S.font.ttf != NULL);
}

void pico_set_grid (int on) {
    S.grid = on;
    _pico_output_present(0);
}

void pico_set_image_crop (Pico_Rect crop) {
    S.image.crop = crop;
}

void pico_set_image_size (Pico_Dim size) {
    S.image.size = size;
}

void pico_set_pan (Pico_Pos pos) {
    S.pan = pos;
}

void pico_set_size (Pico_Dim phy, Pico_Dim log) {
    // physical
    {
        if (phy.x==PICO_SIZE_KEEP.x && phy.y==PICO_SIZE_KEEP.y) {
            // keep
        } else if (phy.x==PICO_SIZE_FULLSCREEN.x && phy.y==PICO_SIZE_FULLSCREEN.y) {
            pico_assert(0 == SDL_SetWindowFullscreen(WIN, SDL_WINDOW_FULLSCREEN_DESKTOP));
            phy = PHY;
        } else {
            pico_assert(0 == SDL_SetWindowFullscreen(WIN, 0));
            SDL_SetWindowSize(WIN, phy.x, phy.y);
        }
    }

    // logical
    if (log.x==PICO_SIZE_KEEP.x && log.y==PICO_SIZE_KEEP.y) {
        // keep
    } else {
        SDL_RenderSetLogicalSize(REN, log.x, log.y);
        TEX = SDL_CreateTexture (
                REN, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                log.x, log.y
        );
        pico_assert(TEX != NULL);

    }

    if (PHY.x==LOG.x || PHY.y==LOG.y) {
        pico_set_grid(0);
    }

    _pico_output_present(0);
}

void pico_set_show (int on) {
    if (on) {
        SDL_ShowWindow(WIN);
        _pico_output_present(0);
    } else {
        SDL_HideWindow(WIN);
    }
}

void pico_set_style (Pico_Style style) {
    S.style = style;
}

void pico_set_title (const char* title) {
    SDL_SetWindowTitle(WIN, title);
}
