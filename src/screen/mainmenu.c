#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "render.h"
#include "input.h"
#include "util.h"
#include "timer.h"
#include "cdda.h"

#include "screen_manager.h"
#include "screen/mainmenu.h"

// Color palette definitions
#define STEPS_PER_COLOR 8
#define NUM_COLORS      5

#define BLACK    0x000000
#define RED      0xe41800
#define ORANGE   0xff8c00
#define YELLOW   0xfee100
#define WHITE    0xfcfbd0

#define PALETTE_SIZE (STEPS_PER_COLOR * (NUM_COLORS - 1)) + 1

// Fire array definitions
#define FIRE_BLOCK_SIZE 1
#define FIRE_ARRAY_WIDTH 20
#define FIRE_ARRAY_HEIGHT 90
#define FIRE_ARRAY_SIZE (FIRE_ARRAY_WIDTH * FIRE_ARRAY_HEIGHT)

// Color constants
const uint32_t COLORS[NUM_COLORS] = {
    BLACK,
    RED,
    ORANGE,
    YELLOW,
    WHITE,
};

// =======================
// Color utility functions
// =======================

uint8_t
lerp_byte(uint8_t start, uint8_t end, int32_t step)
{
    int32_t fstart = ((int32_t)start) << 12;
    int32_t fend = ((int32_t)end) << 12;
    int32_t fresult = fstart + (((fend - fstart) * step) >> 12);
    return (uint8_t)(fresult >> 12);
}

uint32_t
lerp_color(uint32_t c1, uint32_t c2, int32_t step)
{
    // Get color bytes
    uint8_t r1 = (c1 & 0xff0000) >> 16;
    uint8_t r2 = (c2 & 0xff0000) >> 16;
    uint8_t g1 = (c1 & 0x00ff00) >> 8;
    uint8_t g2 = (c2 & 0x00ff00) >> 8;
    uint8_t b1 = (c1 & 0x0000ff);
    uint8_t b2 = (c2 & 0x0000ff);

    return (
        (uint32_t)lerp_byte(r1, r2, step) << 16
        | (uint32_t)lerp_byte(g1, g2, step) << 8
        | (uint32_t)lerp_byte(b1, b2, step));
}

void
generate_palette(uint32_t *palette)
{
    int32_t steps_per_color_fixnum = (STEPS_PER_COLOR << 12);
    palette[0] = BLACK;
    int n_color = 1;
    for(int ci = 0; ci < NUM_COLORS - 1; ci++) {
        uint32_t c1 = COLORS[ci];
        uint32_t c2 = COLORS[ci + 1];
        for(int32_t step = 0; step < STEPS_PER_COLOR; step++) {
            // actual_step = (step + 0.5) / STEPS_PER_COLOR
            int32_t actual_step = (step << 12) + (ONE >> 1);
            actual_step <<= 12; actual_step /= steps_per_color_fixnum;
            palette[n_color] = lerp_color(c1, c2, actual_step);
            n_color++;
        }
    }
}

#define setColor(prim, c)                            \
    setRGB0(prim,                                    \
            (c & 0xff0000) >> 16,                    \
            (c & 0x00ff00) >> 8,                     \
            (c & 0x0000ff));

// =====================
// Main Menu data struct
// =====================

typedef struct {
    uint32_t num_tiles;
    int32_t   logo_vy;
    int16_t   logo_x;
    uint8_t   logo_w;
    uint8_t   logo_h;
    uint8_t   toggle_fire;
    uint8_t   toggle_palette;
    uint8_t   toggle_hud;
    char      text_buffer[256];
    uint32_t  palette[PALETTE_SIZE];
    uint8_t   fire_array[FIRE_ARRAY_SIZE];
} mainmenu_data;

// =============================
// Set fire array state (on/off)
// =============================

void
set_fire(mainmenu_data *data, int state)
{
    uint8_t color_idx = state ? (PALETTE_SIZE - 1) : 0;
    for(int i = 0; i < FIRE_ARRAY_WIDTH; i++) {
        int idx = (FIRE_ARRAY_SIZE - FIRE_ARRAY_WIDTH) + i;
        data->fire_array[idx] = color_idx;
        // Randomly toggle upper squares
        if(rand() % 2) {
            data->fire_array[idx - FIRE_ARRAY_WIDTH] = color_idx;
        }
    }
}

// ===============================
// Palette and fire draw functions
// ===============================

void
draw_palette(uint32_t *palette)
{
    int16_t y = 60;
    int16_t x = 10;
    for(int16_t i = 0; i < PALETTE_SIZE; i++) {
        TILE_8* tile = get_next_prim();
        increment_prim(sizeof(TILE_8));
        setTile8(tile);
        setXY0(tile, x, y);
        setColor(tile, palette[i]);
        sort_prim(tile, 0);
        x += 10;
    }
}

void
draw_fire_array(mainmenu_data *data)
{
    uint8_t *array = data->fire_array;
    uint32_t *palette = data->palette;

    data->num_tiles = 0;

    // Setup draw area and draw offset.
    // Operations are disposed backwards as this is
    // how the ordering table is sorted
    FILL      *pfill;
    DR_AREA   *parea;
    DR_OFFSET *poffs;
    RECT      rect;

    setRECT(&rect,
            320, 0,
            FIRE_ARRAY_WIDTH * FIRE_BLOCK_SIZE,
            FIRE_ARRAY_HEIGHT * FIRE_BLOCK_SIZE);


    // Clear offscreen area (OTZ = 6)
    {
        pfill = (FILL *)get_next_prim();
        increment_prim(sizeof(FILL));
        setFill(pfill);
        setXY0(pfill, rect.x, rect.y);
        setWH(pfill, rect.w, rect.h);
        setRGB0(pfill, 0, 0, 0);
        sort_prim(pfill, 6);

        // Setup draw area primitive
        parea = (DR_AREA *)get_next_prim();
        increment_prim(sizeof(DR_AREA));
        setDrawArea(parea, &rect);
        sort_prim(parea, 6);

        // Setup draw offset
        poffs = (DR_OFFSET *)get_next_prim();
        increment_prim(sizeof(DR_OFFSET));
        setDrawOffset(poffs, rect.x, rect.y);
        sort_prim(poffs, 6);
    }

    // Draw fire matrix offscreen (OTZ = 5)
    int16_t vx = 0;
    int16_t vy = 0;
    for(int row = 0; row < FIRE_ARRAY_HEIGHT; row++) {
        vx = 0;
        for(int col = 0; col < FIRE_ARRAY_WIDTH; col++) {
            int idx = (row * FIRE_ARRAY_WIDTH) + col;
            if(array[idx] != 0) {
                uint32_t color = palette[array[idx]];
                TILE *tile = get_next_prim();
                increment_prim(sizeof(TILE));
                setTile(tile);
                setWH(tile, FIRE_BLOCK_SIZE, FIRE_BLOCK_SIZE);
                setXY0(tile, vx, vy);
                setColor(tile, color);
                sort_prim(tile, 5);
                data->num_tiles++;
            }
            vx += FIRE_BLOCK_SIZE;
        }
        vy += FIRE_BLOCK_SIZE;
    }


    // Restore draw area (OTZ = 4)
    {
        RECT *clip = render_get_buffer_clip();

        parea = (DR_AREA *)get_next_prim();
        increment_prim(sizeof(DR_AREA));
        setDrawArea(parea, clip);
        sort_prim(parea, 4);

        poffs = (DR_OFFSET *)get_next_prim();
        increment_prim(sizeof(DR_OFFSET));
        setDrawOffset(poffs, clip->x, clip->y);
        sort_prim(poffs, 4);
    }


    // Blit fire onto screen. Use a few sprites.
    vx = 0;
    vy = SCREEN_YRES - (FIRE_ARRAY_HEIGHT * FIRE_BLOCK_SIZE);
    for(vx = 0; vx < SCREEN_XRES; vx += rect.w) {
        SPRT *sprt = (SPRT *)get_next_prim();
        increment_prim(sizeof(SPRT));
        setSprt(sprt);
        setUV0(sprt, 0, 0);
        setXY0(sprt, vx, vy);
        setWH(sprt, rect.w, rect.h);
        setRGB0(sprt, 128, 128, 128);
        sort_prim(sprt, 0);
    }

    // Setup fire texture page
    DR_TPAGE *tpage = get_next_prim();
    increment_prim(sizeof(DR_TPAGE));
    setDrawTPage(tpage, 0, 1, getTPage(2, 1, rect.x, rect.y));
    sort_prim(tpage, 0);
}

// =========
// Doom logo
// =========

void
draw_logo(mainmenu_data *data)
{
    POLY_FT4 *poly = (POLY_FT4 *)get_next_prim();
    increment_prim(sizeof(POLY_FT4));
    setPolyFT4(poly);
    setXYWH(poly, data->logo_x, data->logo_vy >> 12, data->logo_w, data->logo_h);
    setUVWH(poly, 0, 0, data->logo_w, data->logo_h);
    setRGB0(poly, 128, 128, 128);
    setTPage(poly, 2, 0, 320, 256); // Manual input
    sort_prim(poly, 1);
}

// ===========================
// Main Menu vanilla functions
// ===========================

void
screen_mainmenu_load()
{
    srand(get_global_frames());
    mainmenu_data *data = screen_alloc(sizeof(mainmenu_data));
    generate_palette(data->palette);

    data->toggle_fire = 1;
    data->toggle_palette = 0;
    data->toggle_hud = 0;

    // zero-fill array
    memset(data->fire_array, 0, FIRE_ARRAY_SIZE * sizeof(uint8_t));

    // Fill last line with last color
    set_fire(data, 1);

    // Load logo
    uint32_t file_length;
    TIM_IMAGE tim;
    uint8_t *file = file_read("\\DOOM.TIM;1", &file_length);
    if(file) {
        load_texture(file, &tim);
        free(file);

        data->logo_x  = (SCREEN_XRES - tim.prect->w) >> 1;
        data->logo_vy = (SCREEN_YRES + CENTERY) << 12;
        data->logo_w  = tim.prect->w;
        data->logo_h  = tim.prect->h;
    }

    cdda_play_track(1);
}

void
screen_mainmenu_unload(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;
    (void)data;

    screen_free();
    cdda_stop();
}

void
spread_fire(uint8_t *fire_array)
{
    for(int x = 0; x < FIRE_ARRAY_WIDTH; x++) {
        for(int y = 1; y < FIRE_ARRAY_HEIGHT; y++) {
            int idx = (y * FIRE_ARRAY_WIDTH) + x;
            int top_idx = ((y - 1) * FIRE_ARRAY_WIDTH) + x;
            if(fire_array[idx] > 0) {
                fire_array[top_idx] = fire_array[idx] - (rand() % 2);
            } else fire_array[top_idx] = 0;
        }
    }
}

void
screen_mainmenu_update(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    if(pad_pressed(PAD_CROSS)) {
        data->toggle_fire ^= 1;
        set_fire(data, data->toggle_fire);
    }

    if(pad_pressed(PAD_SQUARE)) {
        data->toggle_palette ^= 1;
    }

    if(pad_pressed(PAD_TRIANGLE)) {
        data->toggle_hud ^= 1;
    }

    // Spread fire every even frame only
    // In practice, animation will work as if it were running @ 30FPS
    if(!(get_global_frames() % 2)) {
        spread_fire(data->fire_array);
    }

    // Raise text
    if(data->logo_vy > (CENTERY >> 1) << 12) {
        data->logo_vy -= ONE >> 1;
    }
}

void
screen_mainmenu_draw(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    if(data->toggle_hud) {
        draw_text(10, 20, 1, data->text_buffer);
        snprintf(data->text_buffer, 256,
                 "Commit:    %s\n"
                 "FPS:       %02d\n"
                 "Particles: %d\n"
                 "Song: Doom intro by Sonic Clang",
                 GIT_COMMIT,
                 get_frame_rate(),
                 data->num_tiles);
    }

    draw_logo(data);
    if(data->toggle_palette) draw_palette(data->palette);
    draw_fire_array(data);
}
