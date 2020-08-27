#ifndef __control_h__
#define __control_h__

#include <stdint.h>
#include <stdbool.h>

#define CONTROL_BUTTON_A      0x80000000
#define CONTROL_BUTTON_B      0x00200000
#define CONTROL_BUTTON_X      0x00010000
#define CONTROL_BUTTON_Y      0x00000040
#define CONTROL_BUTTON_START  0x00000800
#define CONTROL_BUTTON_SELECT 0x00000400

#define CONTROL_TRIGGER_LEFT  0x00000100
#define CONTROL_TRIGGER_RIGHT 0x20000000

#define CONTROL_DPAD_UP       0x00100000
#define CONTROL_DPAD_DOWN     0x08000000
#define CONTROL_DPAD_LEFT     0x10000000
#define CONTROL_DPAD_RIGHT    0x00040000

typedef struct {
	bool changed;
	bool pressed;
} control_state;

void          control_init();
void          control_term();
void          control_poll();
control_state control_check(uint32_t inControl);
void          control_lock(uint32_t inDuration);

#endif
