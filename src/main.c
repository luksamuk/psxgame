#include <stdlib.h>
#include <stdio.h>
#include <psxcd.h>

#include "render.h"
#include "input.h"
#include "screen_manager.h"

int
main(void)
{
    render_init();
    pad_init();
    screen_init();
    CdInit();
    printf("Hello world! From console\n");

    screen_change(SCREEN_MAINMENU);
    
    while(1) {
        pad_update();
        screen_update();
        screen_draw();
        swap_buffers();
    }
    return 0;
}
