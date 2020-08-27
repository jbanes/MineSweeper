#include <stdlib.h>
//#include <entry.h>
#include "display.h"
#include "SDL.h"

SDL_Surface* screen;

void display_init() {
    screen = SDL_SetVideoMode(display_width, display_height, 16, SDL_DOUBLEBUF | SDL_HWSURFACE);
}

void* display_addr() {
    return screen->pixels;
}

void display_clear(uint8_t inRed, uint8_t inGreen, uint8_t inBlue) {
	uint32_t* tempPtr = (uint32_t*)display_addr();
	uint32_t* tempEnd = (uint32_t*)((uintptr_t)tempPtr + (display_width * display_height << 1));
	uint32_t  tempColor = (inBlue >> 3) | ((inGreen & 0xFC) << 3) | ((inRed & 0xF8) << 10);
	tempColor |= (tempColor << 16);
	while(tempPtr < tempEnd) {
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
		*(tempPtr++) = tempColor;
	}
}

void display_flip() {
    SDL_Flip(screen);
}

void display_release() {
    SDL_FreeSurface(screen);
}