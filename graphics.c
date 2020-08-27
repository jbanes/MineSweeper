#include <stdlib.h>
#include <stdio.h>
//#include <fsys.h>
#include "graphics.h"
#include "display.h"

inline uint16_t _abs_s16(int16_t inVal) { return (inVal >= 0 ? inVal : -inVal); }

inline gfx_color gfx_color_rgb(uint8_t inRed, uint8_t inGreen, uint8_t inBlue) {
		return ((inRed & 0xF8) << 8) | ((inGreen & 0xFC) << 3) | ((inBlue & 0xF8) >> 3);
}

inline gfx_color gfx_color_rand(uint8_t inRed, uint8_t inGreen, uint8_t inBlue) {
	return gfx_color_rgb((inRed + (rand() % (256 - inRed))), (inGreen + (rand() % (256 - inGreen))), (inBlue + (rand() % (256 - inBlue))));
}



gfx_texture  gfx_frame_buffer = { NULL, display_width, display_height };
gfx_texture* gfx_render_target = &gfx_frame_buffer;

bool gfx_init() {
        display_init();
        
	gfx_frame_buffer.address = display_addr();
	gfx_frame_buffer.width   = display_width;
	gfx_frame_buffer.height  = display_height;
        
	return true;
}

void gfx_term() {
	gfx_frame_buffer.address = NULL;
}



bool gfx_render_target_set(gfx_texture* inTexture) {
	if(inTexture == NULL) {
		gfx_render_target = &gfx_frame_buffer;
		return true;
	}

	if((inTexture->width & 0xF) != 0)
		return false;
	gfx_render_target = inTexture;
	return true;
}

void gfx_render_target_clear(gfx_color inColor) {
	uint32_t  tempPixelColor = (inColor << 16) | inColor;
	uint32_t* tempLineStart = (uint32_t*)gfx_render_target->address;
	uint32_t* tempBlockEnd;
	tempBlockEnd = (uint32_t*)((uintptr_t)gfx_render_target->address + (gfx_render_target->width * gfx_render_target->height * 2));
	while(tempLineStart < tempBlockEnd) {
		*(tempLineStart++) = tempPixelColor;
		*(tempLineStart++) = tempPixelColor;
		*(tempLineStart++) = tempPixelColor;
		*(tempLineStart++) = tempPixelColor;
	}
}

void gfx_render_target_swap() {
	if(gfx_render_target != &gfx_frame_buffer)
		return;
	display_flip();
	gfx_frame_buffer.address = display_addr();
}



gfx_texture* gfx_tex_load_tga(const char* inPath) {
	if(inPath == NULL)
		return NULL;

	FILE* tempFile = fopen(inPath, "rb");
	if(tempFile == NULL)
		return NULL;

	uint8_t  tga_ident_size;
	uint8_t  tga_color_map_type;
	uint8_t  tga_image_type;
	uint16_t tga_color_map_start;
	uint16_t tga_color_map_length;
	uint8_t  tga_color_map_bpp;
	uint16_t tga_origin_x;
	uint16_t tga_origin_y;
	uint16_t tga_width;
	uint16_t tga_height;
	uint8_t  tga_bpp;
	uint8_t  tga_descriptor;

	fread(&tga_ident_size, 1, 1, tempFile);
	fread(&tga_color_map_type, 1, 1, tempFile);
	fread(&tga_image_type, 1, 1, tempFile);
	fread(&tga_color_map_start, 2, 1, tempFile);
	fread(&tga_color_map_length, 2, 1, tempFile);
	fread(&tga_color_map_bpp, 1, 1, tempFile);
	fread(&tga_origin_x, 2, 1, tempFile);
	fread(&tga_origin_y, 2, 1, tempFile);
	fread(&tga_width, 2, 1, tempFile);
	fread(&tga_height, 2, 1, tempFile);
	fread(&tga_bpp, 1, 1, tempFile);
	fread(&tga_descriptor, 1, 1, tempFile);

	gfx_texture* tempTexture = (gfx_texture*)malloc(sizeof(gfx_texture) + (tga_width * tga_height * 2));
	if(tempTexture == NULL) {
		fclose(tempFile);
		return NULL;
	}
	tempTexture->address = (void*)((uintptr_t)tempTexture + sizeof(gfx_texture));
	tempTexture->width = tga_width;
	tempTexture->height = tga_height;

	uintptr_t i;
	uint8_t tempColor[3];
	uint16_t* tempTexPtr = (uint16_t*)tempTexture->address;
	for(i = 0; i < (tga_width * tga_height); i++) {
		fread(&tempColor[2], 1, 1, tempFile);
		fread(&tempColor[1], 1, 1, tempFile);
		fread(&tempColor[0], 1, 1, tempFile);
		tempTexPtr[i] = gfx_color_rgb(tempColor[0], tempColor[1], tempColor[2]);
	}
	fclose(tempFile);

	return tempTexture;
}

bool gfx_tex_save_tga(const char* inPath, gfx_texture* inTexture) {
	if((inPath == NULL) || (inTexture == NULL) || (inTexture->address == NULL))
		return false;

	FILE* tempFile = fopen(inPath, "wb");
	if(tempFile == NULL)
		return false;

	uint8_t  tga_ident_size = 0;
	uint8_t  tga_color_map_type = 0;
	uint8_t  tga_image_type = 2;
	uint16_t tga_color_map_start = 0;
	uint16_t tga_color_map_length = 0;
	uint8_t  tga_color_map_bpp = 0;
	uint16_t tga_origin_x = 0;
	uint16_t tga_origin_y = 0;
	int16_t  tga_width = inTexture->width;
	int16_t  tga_height = inTexture->height;
	uint8_t  tga_bpp = 24;
	uint8_t  tga_descriptor = 0x20;

	fwrite(&tga_ident_size, 1, 1, tempFile);
	fwrite(&tga_color_map_type, 1, 1, tempFile);
	fwrite(&tga_image_type, 1, 1, tempFile);
	fwrite(&tga_color_map_start, 2, 1, tempFile);
	fwrite(&tga_color_map_length, 2, 1, tempFile);
	fwrite(&tga_color_map_bpp, 1, 1, tempFile);
	fwrite(&tga_origin_x, 2, 1, tempFile);
	fwrite(&tga_origin_y, 2, 1, tempFile);
	fwrite(&tga_width, 2, 1, tempFile);
	fwrite(&tga_height, 2, 1, tempFile);
	fwrite(&tga_bpp, 1, 1, tempFile);
	fwrite(&tga_descriptor, 1, 1, tempFile);

	uintptr_t i;
	uint8_t tempColor[3];
	uint16_t* tempTexPtr = (uint16_t*)inTexture->address;
	for(i = 0; i < (tga_width * tga_height); i++) {
		tempColor[0]  = ((tempTexPtr[i] >> 8) & 0xF8);
		tempColor[0] |= (tempColor[0] >> 5);
		tempColor[1]  = ((tempTexPtr[i] >> 3) & 0xFC);
		tempColor[1] |= (tempColor[1] >> 6);
		tempColor[2]  = ((tempTexPtr[i] << 3) & 0xF8);
		tempColor[2] |= (tempColor[2] >> 5);
		fwrite(&tempColor[2], 1, 1, tempFile);
		fwrite(&tempColor[1], 1, 1, tempFile);
		fwrite(&tempColor[0], 1, 1, tempFile);
	}
	fclose(tempFile);

	return true;
}



void gfx_point_draw(int16_t inX, int16_t inY, gfx_color inColor) {
	uint16_t* tempPtr = (uint16_t*)(gfx_render_target->address);
	tempPtr[(inY * gfx_render_target->width) + inX] = inColor;
}

void gfx_line_draw(int16_t inX0, int16_t inY0, int16_t inX1, int16_t inY1, gfx_color inColor) {
	int16_t tempDx = inX1 - inX0;
	int16_t tempDy = inY1 - inY0;
	bool tempSteep = (_abs_s16(tempDy) >= _abs_s16(tempDx));
	if(tempSteep) {
		inX0 = inX0 ^ inY0;
		inY0 = inX0 ^ inY0;
		inX0 = inX0 ^ inY0;
		inX1 = inX1 ^ inY1;
		inY1 = inX1 ^ inY1;
		inX1 = inX1 ^ inY1;
		tempDx = inX1 - inX0;
		tempDy = inY1 - inY0;
	}
	int16_t tempXStep = 1;
	if(tempDx < 0) {
		tempXStep = -1;
		tempDx = -tempDx;
	}
	int16_t tempYStep = 1;
	if(tempDy < 0) {
		tempYStep = -1;
		tempDy = -tempDy;
	}
	int16_t tempTwoDy = tempDy * 2;
	int16_t tempTwoDyTwotempDx = tempTwoDy - (tempDx * 2);
	int16_t tempErr = tempTwoDy - tempDx;

	uint16_t* tempPtr = (uint16_t*)gfx_render_target->address;
	int16_t tempX, tempY;
	if(tempSteep) {
		for(tempX = inX0, tempY = inY0; tempX != inX1; tempX += tempXStep) {
			tempPtr[(tempX * gfx_render_target->width) + tempY] = inColor;
			if(tempErr > 0) {
				tempErr += tempTwoDyTwotempDx;
				tempY += tempYStep;
			} else
				tempErr += tempTwoDy;
		}
	} else {
		for(tempX = inX0, tempY = inY0; tempX != inX1; tempX += tempXStep) {
			tempPtr[(tempY * gfx_render_target->width) + tempX] = inColor;
			if(tempErr > 0) {
				tempErr += tempTwoDyTwotempDx;
				tempY += tempYStep;
			} else
				tempErr += tempTwoDy;
		}
	}
}

void gfx_rect_draw(int16_t inX, int16_t inY, int16_t inWidth, int16_t inHeight, gfx_color inColor) {
	uint16_t* tempBuffer = (uint16_t*)gfx_render_target->address;
	uintptr_t i, j;
	j = 0;
	for(j = inY, i = (inX > 0 ? inX : 0); (i < gfx_render_target->width) && (i < (inX + inWidth)); i++)
		tempBuffer[(j * gfx_render_target->width) + i] = inColor;
	for(j = (inY + 1); (j < gfx_render_target->height) && (j < (inY + inHeight - 1)); j++) {
		if(inX >= 0)
			tempBuffer[(j * gfx_render_target->width) + inX] = inColor;
		if((inX + inWidth - 1) < gfx_render_target->width)
			tempBuffer[(j * gfx_render_target->width) + (inX + inWidth - 1)] = inColor;
	}
	for(j = (inY + inHeight - 1), i = (inX > 0 ? inX : 0); (i < gfx_render_target->width) && (i < (inX + inWidth)); i++)
		tempBuffer[(j * gfx_render_target->width) + i] = inColor;
}

void gfx_rect_fill_draw(int16_t inX, int16_t inY, int16_t inWidth, int16_t inHeight, gfx_color inColor) {
	if(inX < 0) {
		inWidth += inX;
		inX = 0;
	}
	if(inY < 0) {
		inHeight += inY;
		inY = 0;
	}
	uint16_t* tempBuffer = (uint16_t*)gfx_render_target->address;
	uintptr_t i, j;
	for(j = inY; (j < gfx_render_target->height) && (j < (inY + inHeight)); j++) {
		for(i = inX; (i < gfx_render_target->width) && (i < (inX + inWidth)); i++) {
			tempBuffer[(j * gfx_render_target->width) + i] = inColor;
		}
	}
}

void gfx_tri_draw(int16_t inX0, int16_t inY0, int16_t inX1, int16_t inY1, int16_t inX2, int16_t inY2, gfx_color inColor) {
	gfx_line_draw(inX0, inY0, inX1, inY1, inColor);
	gfx_line_draw(inX1, inY1, inX2, inY2, inColor);
	gfx_line_draw(inX2, inY2, inX0, inY0, inColor);
}



void gfx_tex_draw(int16_t inX, int16_t inY, gfx_texture* inTexture) {
	if(inTexture == NULL)
		return;

	uint16_t* tempBuffer = (uint16_t*)gfx_render_target->address;
	uint16_t* tempTexture = (uint16_t*)inTexture->address;

	int16_t x, y, i, j;
	for(j = 0, y = inY; (j < inTexture->height) && (y < gfx_render_target->height); j++, y++) {
		for(i = 0, x = inX; (i < inTexture->width) && (x < gfx_render_target->width); i++, x++) {
			tempBuffer[(y * gfx_render_target->width) + x] = tempTexture[(j * inTexture->width) + i];
		}
	}
}



uint16_t gfx_font_width(gfx_texture* inFont, char* inString) {
	if((inFont == NULL) || (inString == NULL))
		return 0;
	uintptr_t i, tempCur, tempMax;
	for(i = 0, tempCur = 0, tempMax = 0; inString[i] != '\0'; i++) {
		if(inString[i] == '\t')
			tempCur += 4;
		else if((inString[i] == '\r') || (inString[i] == '\n'))
			tempCur = 0;
		else
			tempCur++;
		if(tempCur > tempMax) tempMax = tempCur;
	}
	tempMax *= (inFont->width >> 4);
	return tempMax;
}

uint16_t gfx_font_height(gfx_texture* inFont) {
	if(inFont == NULL)
		return 0;
	return (inFont->height >> 4);
}

void gfx_font_print_char(int16_t inX, int16_t inY, gfx_texture* inFont, char inChar) {
	char tempStr[2] = { inChar, '\0' };
	gfx_font_print(inX, inY, inFont, tempStr);
}

void gfx_font_print(int16_t inX, int16_t inY, gfx_texture* inFont, char* inString) {
	if((inFont == NULL) || (inString == NULL))
		return;

	uint16_t* tempBuffer = (uint16_t*)gfx_render_target->address;
	uint16_t* tempFont = (uint16_t*)inFont->address;
	char* tempChar = inString;
	int16_t tempX = inX;
	int16_t tempY = inY;
	uintptr_t i, j, x, y;

	for(tempChar = inString; *tempChar != '\0'; tempChar++) {
		if(*tempChar == ' ') {
			tempX += (inFont->width >> 4);
			continue;
		}
		if(*tempChar == '\t') {
			tempX += ((inFont->width >> 4) << 2);
			continue;
		}
		if(*tempChar == '\r') {
			tempX = inX;
			continue;
		}
		if(*tempChar == '\n') {
			tempX = inX;
			tempY += (inFont->height >> 4);
			continue;
		}
		for(j = ((*tempChar >> 4) * (inFont->height >> 4)), y = tempY; (j < (((*tempChar >> 4) + 1) * (inFont->height >> 4))) && (y < gfx_render_target->height); j++, y++) {
			for(i = ((*tempChar & 0x0F) * (inFont->width >> 4)), x = tempX; (i < (((*tempChar & 0x0F) + 1) * (inFont->width >> 4))) && (x < gfx_render_target->width); i++, x++) {
				tempBuffer[(y * gfx_render_target->width) + x] |= tempFont[(j * inFont->width) + i];
			}
		}
		tempX += (inFont->width >> 4);
	}
}

void gfx_font_print_center(int16_t inY, gfx_texture* inFont, char* inString) {
	int16_t tempX = (gfx_render_target->width - gfx_font_width(inFont, inString)) >> 1;
	gfx_font_print(tempX, inY, inFont, inString);
}

void gfx_font_print_fromright(int16_t inX, int16_t inY, gfx_texture* inFont, char* inString) {
	int16_t tempX = inX - gfx_font_width(inFont, inString);
	gfx_font_print(tempX, inY, inFont, inString);
}
