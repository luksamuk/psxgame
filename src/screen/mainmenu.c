#include <stdio.h>
#include "render.h"
#include "input.h"

#include "screen_manager.h"
#include "screen/mainmenu.h"

typedef struct {
    char text_buffer[80];
    int32_t counter;
} mainmenu_data;

void
screen_mainmenu_load()
{
    mainmenu_data *data = screen_alloc(sizeof(mainmenu_data));
    data->counter = 0;
}

void
screen_mainmenu_unload(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;
}

void
screen_mainmenu_update(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;

    if(pad_pressed(PAD_UP)) data->counter++;
    if(pad_pressed(PAD_DOWN)) data->counter--;
    
    snprintf(data->text_buffer, 80, "Hello, world! Counter: %d", data->counter);
}

void
screen_mainmenu_draw(void *d)
{
    mainmenu_data *data = (mainmenu_data *)d;
    
    draw_text(10, 10, 0, data->text_buffer);
}
