#ifndef __display_h__
#define __display_h__

#include <stdbool.h>
#include <stdint.h>

#define display_width  320
#define display_height 240

void display_init();
void* display_addr();
void  display_clear(uint8_t inRed, uint8_t inGreen, uint8_t inBlue);
void  display_flip();
void display_release();

#endif
