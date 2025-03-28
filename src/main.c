#include <stdlib.h>
#include <stdio.h>

#include "render.h"

int
main(void)
{
    render_init();
    printf("Hello world!\n");

    while(1) {
        draw_text(10, 10, 0, "Hello, world!");
        swap_buffers();
    }
    return 0;
}
