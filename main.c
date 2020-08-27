#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "graphics.h"
#include "display.h"
#include "control.h"
#include "timer.h"
#include "SDL.h"

#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)

char* gamePathInit(const char* inPath);
void  gameScreenshot();
void  gameDialog(char* inMessage);

void gridCreate(uint32_t inWidth, uint32_t inHeight, uint32_t inMines);
void gridDelete();
void gridPlaceMines(uint32_t inCount);
uint8_t gridRand(uint8_t inBound);
void gridPlaceMine();
void gridDraw();
bool gridMined(int inX, int inY);
int  gridMarked(int inX, int inY);
bool gridCovered(int inX, int inY);
int  gridNumber(int inX, int inY);
int  gridSelect(int inX, int inY);
void gridMark(int inX, int inY);
int  gridCondition();

void levelCreate();
void levelWin();
void levelLose();

void hudDraw();



gfx_texture* fontDefault = NULL;

uint32_t gridWidth  = 0;
uint32_t gridHeight = 0;
uint8_t* gridData   = NULL;
int      gridMines  = 0;

uint32_t cursorX = 0;
uint32_t cursorY = 0;

timer*   gameTimer    = NULL;
uint32_t gameTime     = 0;
uint32_t gameTickRate = 1;
bool     gameRunning  = true;
uint32_t gameLevel	  = 0;
char     gamePath[256];



char* gamePathInit(const char* inPath) {
	uintptr_t i, j;
	for(i = 0, j = 0; inPath[i] != '\0'; i++) {
		if((inPath[i] == '\\') || (inPath[i] == '/'))
			j = i + 1;
	}
	strncpy(gamePath, inPath, j);
	return gamePath;
}

void gameScreenshot() {
	char tempString[256];
	unsigned long int tempNumber = 0;
	FILE* tempFile;
	while(true) {
		sprintf(tempString, "%sscreenshot%lu.tga", gamePath, tempNumber);
		tempFile = fopen(tempString, "rb");
		if(tempFile == NULL)
			break;
		fclose(tempFile);
		tempNumber++;
	}
	gfx_tex_save_tga(tempString, gfx_render_target);
}

void gameDialog(char* inMessage) {
	int tempX, tempY;
	int tempWidth, tempHeight;
	tempWidth = (gfx_font_width(fontDefault, inMessage) + 16);
	tempWidth = max(tempWidth, (gfx_font_width(fontDefault, "Press [A/B]") + 16));
	tempHeight = ((gfx_font_height(fontDefault) * 3) + 16);
	tempX = (display_width - tempWidth) >> 1;
	tempY = (display_height - tempHeight) >> 1;

	control_lock(timer_resolution / 2);

	while(gameRunning) {

		gridDraw();
		hudDraw();

		gfx_rect_fill_draw(tempX, tempY, tempWidth, tempHeight, gfx_color_rgb(0x7F, 0x7F, 0x7F));
		gfx_rect_draw(tempX, tempY, tempWidth, tempHeight, gfx_color_rgb(0xFF, 0xFF, 0xFF));

		gfx_font_print_center((tempY + 8), fontDefault, inMessage);
		gfx_font_print_center((tempY + (2 * gfx_font_height(fontDefault)) + 8), fontDefault, "Press [A/B]");

		gfx_render_target_swap();

		control_poll();
		if(control_check(CONTROL_BUTTON_B).pressed && control_check(CONTROL_BUTTON_B).changed)
			break;
		if(control_check(CONTROL_BUTTON_A).pressed && control_check(CONTROL_BUTTON_A).changed)
			break;
//		if(control_check(CONTROL_BUTTON_SELECT).pressed && control_check(CONTROL_BUTTON_SELECT).changed) {
//			gameScreenshot();
//			gameDialog("Screenshot taken.");
//		}
	}
}

void gridCreate(uint32_t inWidth, uint32_t inHeight, uint32_t inMines) {
	if((inWidth == 0) || (inHeight == 0))
		return;
	if((inWidth << 4) > display_width)
		inWidth = (display_width >> 4);
	if((inHeight << 4) > (display_height - 16))
		inHeight = (display_height >> 4) - 1;
	gridDelete();
	gridData = (uint8_t*)malloc(inWidth * inHeight);
	if(gridData == NULL) {
		gameDialog("Error: Couldn't allocate grid.");
		return;
	}
	gridWidth = inWidth;
	gridHeight = inHeight;
	memset(gridData, 1, (inWidth * inHeight));
	gridMines = 0;
	gridPlaceMines(inMines);
}

void gridDelete() {
	if(gridData != NULL)
		free(gridData);
	gridData = NULL;
	gridWidth = 0;
	gridHeight = 0;
	gridMines = 0;
}

void gridPlaceMines(uint32_t inCount) {
	if(inCount > (gridWidth * gridHeight))
		inCount = (gridWidth * gridHeight);
	uint32_t i;
	for(i = 0; i < inCount; i++)
		gridPlaceMine();
}

uint8_t gridRand(uint8_t inBound) {
	uint32_t tempRand = rand();
	tempRand = (tempRand >> 24) ^ ((tempRand >> 16) & 0xFF) ^ ((tempRand >> 8) & 0xFF) ^ (tempRand & 0xFF);
	return (tempRand % inBound);
}

void gridPlaceMine() {
	if((gridData == NULL) || (gridWidth == 0) || (gridHeight == 0))
		return;
	if(gridMines == (gridWidth * gridHeight))
		return;
	uint32_t tempX = gridRand(gridWidth);
	uint32_t tempY = gridRand(gridHeight);
	uint32_t i = 0;
	while(gridMined(tempX, tempY)) {
		tempX = gridRand(gridWidth);
		tempY = gridRand(gridHeight);
		i++;
		if(i >= ((gridWidth * gridHeight) << 2)) {
			gameDialog("Error: Couldn't place mine, not random enough.");
			return;
		}
	}
	gridData[(tempY * gridWidth) + tempX] |= 2;
	gridMines++;
}

void gridDraw() {
	if((gridData == NULL) || (gridWidth == 0) || (gridHeight == 0))
		return;

	uint32_t tempX = ((display_width - (gridWidth << 4)) >> 1);
	uint32_t tempY = 16 + (((display_height - 16) - (gridHeight << 4)) >> 1);

	uint32_t i; uint32_t j; int tempNumber;
	for(j = 0; j < gridHeight; j++) {
		for(i = 0; i < gridWidth; i++) {
			if(gridCovered(i, j)) {
				if((i == cursorX) && (j == cursorY))
					gfx_rect_fill_draw((tempX + (i << 4)), (tempY + (j << 4)), 16, 16, gfx_color_rgb(0x00, 0x7F, 0x00));
				else
					gfx_rect_fill_draw((tempX + (i << 4)), (tempY + (j << 4)), 16, 16, gfx_color_rgb(0x3F, 0x3F, 0x3F));
				int tempGridMark = gridMarked(i, j);
				if(tempGridMark == 1) {
					gfx_font_print_char((tempX + (i << 4) + 5), (tempY + (j << 4) + 5), fontDefault, 'X');
				} else if (tempGridMark == 2) {
					gfx_font_print_char((tempX + (i << 4) + 5), (tempY + (j << 4) + 5), fontDefault, '?');
				}
			} else {
				if(gridMined(i, j)) {
					gfx_rect_fill_draw((tempX + (i << 4)), (tempY + (j << 4)), 16, 16, gfx_color_rgb(0xFF, 0x00, 0x00));
				} else {
					if((i == cursorX) && (j == cursorY))
						gfx_rect_fill_draw((tempX + (i << 4)), (tempY + (j << 4)), 16, 16, gfx_color_rgb(0x00, 0xBF, 0x00));
					else
						gfx_rect_fill_draw((tempX + (i << 4)), (tempY + (j << 4)), 16, 16, gfx_color_rgb(0xBF, 0xBF, 0xBF));
					tempNumber = gridNumber(i, j);
					if(tempNumber > 0)
						gfx_font_print_char((tempX + (i << 4) + 5), (tempY + (j << 4) + 5), fontDefault, (tempNumber + '0'));
				}
			}
		}
	}

	for(i = 0; i <= gridWidth; i++)
		gfx_line_draw((tempX + (i << 4)), tempY, (tempX + (i << 4)), (tempY + (gridHeight << 4) + 1), gfx_color_rgb(0x7F, 0x7F, 0x7F));
	for(i = 0; i <= gridHeight; i++)
		gfx_line_draw(tempX, (tempY + (i << 4)), (tempX + (gridWidth << 4) + 1), (tempY + (i << 4)), gfx_color_rgb(0x7F, 0x7F, 0x7F));

	gfx_rect_draw((tempX + (cursorX << 4)), (tempY + (cursorY << 4)), 17, 17, gfx_color_rgb(0x00, 0xFF, 0x00));
}

bool gridMined(int inX, int inY) {
	if((gridData == NULL) || (gridWidth == 0) || (gridHeight == 0))
		return false;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return false;
	if((inX < 0) || (inY < 0))
		return false;

	return ((gridData[(inY * gridWidth) + inX] & 2) == 2);
}

int gridMarked(int inX, int inY) {
	if((gridData == NULL) || (gridWidth == 0) || (gridHeight == 0))
		return -1;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return -1;
	if((inX < 0) || (inY < 0))
		return -1;

	if(!(gridData[(inY * gridWidth) + inX] & 4))
		return 0;
	if((gridData[(inY * gridWidth) + inX] & 8))
		return 2;
	return 1;
}

bool gridCovered(int inX, int inY) {
	if(!gridData || !gridWidth || !gridHeight)
		return -1;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return -1;
	if((inX < 0) || (inY < 0))
		return -1;

	return ((gridData[(inY * gridWidth) + inX] & 1) == 1);
}

int gridNumber(int inX, int inY) {
	if(!gridData || !gridWidth || !gridHeight)
		return -1;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return -1;
	if((inX < 0) || (inY < 0))
		return -1;

	int tempOut = 0;

	tempOut += (gridMined((inX - 1), (inY - 1)) ? 1 : 0);
	tempOut += (gridMined((inX - 1), inY) ? 1 : 0);
	tempOut += (gridMined((inX - 1), (inY + 1)) ? 1 : 0);

	tempOut += (gridMined(inX, (inY - 1)) ? 1 : 0);
	tempOut += (gridMined(inX, (inY + 1)) ? 1 : 0);

	tempOut += (gridMined((inX + 1), (inY - 1)) ? 1 : 0);
	tempOut += (gridMined((inX + 1), inY) ? 1 : 0);
	tempOut += (gridMined((inX + 1), (inY + 1)) ? 1 : 0);

	return tempOut;
}

int gridSelect(int inX, int inY) {
	if(!gridData || !gridWidth || !gridHeight)
		return -1;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return -1;
	if((inX < 0) || (inY < 0))
		return -1;

	if(!(gridCovered(inX, inY)))
		return 0;

	if(gridMarked(inX, inY) != 0)
		return 0;

	gridData[(inY * gridWidth) + inX] ^= 1;

	if(gridMined(inX, inY))
		return 1;

	if(gridNumber(inX, inY) == 0) {
		gridSelect((inX - 1), inY);
		gridSelect((inX - 1), (inY - 1));
		gridSelect((inX - 1), (inY + 1));

		gridSelect(inX, (inY - 1));
		gridSelect(inX, (inY + 1));

		gridSelect((inX + 1), inY);
		gridSelect((inX + 1), (inY - 1));
		gridSelect((inX + 1), (inY + 1));
	}

	return 0;
}

void gridMark(int inX, int inY) {
	if(!gridData || !gridWidth || !gridHeight)
		return;
	if((inX >= gridWidth) || (inY >= gridHeight))
		return;
	if((inX < 0) || (inY < 0))
		return;

	if(!(gridCovered(inX, inY)))
		return;

	if(gridData[(inY * gridWidth) + inX] & 8) {
		gridData[(inY * gridWidth) + inX] &= ~12;
		return;
	}
	if(gridData[(inY * gridWidth) + inX] & 4) {
		gridData[(inY * gridWidth) + inX] |= 8;
		gridMines++;
		return;
	}

	gridData[(inY * gridWidth) + inX] |= 4;
	gridMines--;
}

int gridCondition() {
	if((gridData == NULL) || (gridWidth == 0) || (gridHeight == 0))
		return -2;

	uint32_t tempTilesLeft = 0;
	uint32_t tempTilesWrong = 0;
	uint32_t i; uint32_t j;
	for(j = 0; j < gridHeight; j++) {
		for(i = 0; i < gridWidth; i++) {
			if(gridCovered(i, j)) {
				if(!gridMined(i, j)) {
					tempTilesLeft++;
					if(gridMarked(i, j) == 1)
						tempTilesWrong++;
				}
			} else if(gridMined(i, j)) {
				return -1;
			}
		}
	}
	if((gridMines == 0) && (tempTilesWrong == 0))
		return 1;
	if(tempTilesLeft == 0)
		return 1;
	return 0;
}



void levelCreate() {
	uint32_t tempDimensions = (3 + gameLevel);
	uint32_t tempMines = ((tempDimensions * tempDimensions) >> 4) + gameLevel;
	if(tempMines == 0)
		tempMines++;
	if(tempMines >= ((tempDimensions * tempDimensions) >> 2))
		tempMines = ((tempDimensions * tempDimensions) >> 2) - 1;
	gridCreate(tempDimensions, tempDimensions, tempMines);
}

void levelWin() {
	gameDialog("You Win!");
	gameLevel++;
	gameTime = 0;
	levelCreate();
}

void levelLose() {
	gameDialog("You Lose!");
	gameTime = 0;
	levelCreate();
}

void hudDraw() {
	gfx_rect_fill_draw(0, 0, display_width, 16, gfx_color_rgb(0x00, 0x00, 0x7F));

	char tempString[256];
	sprintf(tempString, "Mines: %i", gridMines);
	gfx_font_print(4, 4, fontDefault, tempString);

	unsigned long int tempSeconds  = (gameTime / timer_resolution);
	unsigned long int tempFraction = ((gameTime % timer_resolution) * 10) / timer_resolution;
	sprintf(tempString, "Time: %lu.%01lu", tempSeconds, tempFraction);
	gfx_font_print_center(4, fontDefault, tempString);

	sprintf(tempString, "Level: %u", (unsigned int)gameLevel);
	gfx_font_print_fromright((display_width - 4), 4, fontDefault, tempString);

	gfx_font_print_fromright((display_width - 2), (display_height - gfx_font_height(fontDefault) - 2), fontDefault, "[QUIT]");
//	gfx_font_print(2, (display_height - gfx_font_height(fontDefault) - 2), fontDefault, "[SCREENSHOT]");
}

int main(int argc, char** argv) {
	gamePathInit(argv[0]);
	int ref = EXIT_SUCCESS;

	//srand(GetTickCount());
	srand(0x12345678);
        SDL_Init(SDL_INIT_VIDEO);
        SDL_WM_SetCaption("MineSweeper", "MineSweeper");

	control_init();
	gfx_init();
	gfx_render_target_clear(gfx_color_rgb(0x00, 0x00, 0x00));
	gfx_render_target_swap();

	char tempString[256];
	sprintf(tempString, "%sfont.tga", gamePath);
	fontDefault = gfx_tex_load_tga(tempString);

	gameTimer = timer_create();
	gameLevel = 1;
	levelCreate();

	uint32_t tempTick = 0;
	int sysref;
	while(gameRunning) {
            gameTime += timer_delta(gameTimer);

            gfx_render_target_clear(gfx_color_rgb(0x00, 0x00, 0x00));
            gridDraw();
            hudDraw();
            gfx_render_target_swap();

	    control_poll();
            
            if(control_check(CONTROL_DPAD_UP).pressed && control_check(CONTROL_DPAD_UP).changed)
                    cursorY -= (cursorY > 0 ? 1 : 0);
            if(control_check(CONTROL_DPAD_DOWN).pressed && control_check(CONTROL_DPAD_DOWN).changed)
                    cursorY += (cursorY < (gridHeight - 1) ? 1 : 0);
            if(control_check(CONTROL_DPAD_LEFT).pressed && control_check(CONTROL_DPAD_LEFT).changed)
                    cursorX -= (cursorX > 0 ? 1 : 0);
            if(control_check(CONTROL_DPAD_RIGHT).pressed && control_check(CONTROL_DPAD_RIGHT).changed)
                    cursorX += (cursorX < (gridWidth - 1) ? 1 : 0);

            if(control_check(CONTROL_BUTTON_A).pressed && control_check(CONTROL_BUTTON_A).changed)
                    gridSelect(cursorX, cursorY);
            if(control_check(CONTROL_BUTTON_B).pressed && control_check(CONTROL_BUTTON_B).changed)
                    gridMark(cursorX, cursorY);

            if(control_check(CONTROL_BUTTON_START).pressed && control_check(CONTROL_BUTTON_START).changed)
                    gameRunning = false;
            if(control_check(CONTROL_BUTTON_SELECT).pressed && control_check(CONTROL_BUTTON_SELECT).changed) {
                    gameScreenshot();
                    gameDialog("Screenshot taken.");
            }

            int tempCondition = gridCondition();
            if(tempCondition < 0) {
                if(tempCondition == -1) levelLose();
                else levelCreate();
            } else if(tempCondition > 0) {
                levelWin();
            }
	}

	gridDelete();
	timer_delete(gameTimer);
	free(fontDefault);
	gfx_term();
	control_term();
        
        // SDL_Free(surface)
        SDL_Quit();

	return 0;
}
