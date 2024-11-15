#include "form.h"

struct Form* create_form(void)
{
    struct Form *form = (struct Form*)malloc(sizeof(struct Form));
    if (form == NULL)
    {
        return NULL;
    }

     // init video and joystick
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

    // get physical devices' resolution
    uint16_t physical_device_res_width = 0;
    uint16_t physical_device_res_height = 0;
    const SDL_DisplayMode* physical_device_display_mode = SDL_GetDesktopDisplayMode(0);
    if (physical_device_display_mode == NULL)
    {
        // Call SDL_GetError() to get the error message
        const char* error_message = SDL_GetError();
        FORM_EMERGENCY_PRINT("Failed to get physical device display mode: %s\n", error_message);
        free(form);
        return NULL;
    }
    physical_device_res_width = physical_device_display_mode->w;
    physical_device_res_height = physical_device_display_mode->h;

    FORM_DEBUG_PRINT("Physical device resolution: %dx%d\n", physical_device_res_width, physical_device_res_height);
    
    // create window (not resizable)
    form->window = SDL_CreateWindow("GameBoy", 160, 144, SDL_WINDOW_MAXIMIZED);
    if (form->window == NULL)
    {
        free(form);
        return NULL;
    }
    // store window surface
    form->surface = SDL_GetWindowSurface(form->window);

    // allocate framebuffer
    form->framebuffer = (uint8_t*)malloc(160 * 144 * sizeof(uint8_t));
    if (form->framebuffer == NULL)
    {
        free(form);
        return NULL;
    }

    return form;
}

void free_form(struct Form* form)
{
    free(form->framebuffer);
    // free window
    SDL_DestroyWindow(form->window);
    // free form
    free(form);
}