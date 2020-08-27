#include <stdlib.h>
#include "control.h"
#include "timer.h"
#include "SDL.h"

typedef struct {
	unsigned long pressed;
	unsigned long released;
	unsigned long status;
} KEY_STATUS;

//extern void _kbd_get_status(KEY_STATUS* ks);
//
KEY_STATUS _control_status[2];
timer*     _control_timer = NULL;
uint32_t   _control_lock = 0;

void control_init() {
//	_kbd_get_status(_control_status);
//	_control_status[1] = _control_status[0];
//	_control_lock = 0;
//	_control_timer = timer_create();
    
    
}

void control_term() {
	timer_delete(_control_timer);
	_control_timer = NULL;
}

void control_poll() {
	uint32_t tempDelta = timer_delta(_control_timer);
        SDL_Event event;
        int mask = 0;

	if(tempDelta >= _control_lock) {
		_control_status[1] = _control_status[0];
		
                while(SDL_PollEvent(&event)) 
                {
                    if(event.key.keysym.sym == SDLK_UP)     mask = CONTROL_DPAD_UP;
                    if(event.key.keysym.sym == SDLK_RIGHT)  mask = CONTROL_DPAD_RIGHT;
                    if(event.key.keysym.sym == SDLK_DOWN)   mask = CONTROL_DPAD_DOWN;
                    if(event.key.keysym.sym == SDLK_LEFT)   mask = CONTROL_DPAD_LEFT;
                    if(event.key.keysym.sym == SDLK_LCTRL)  mask = CONTROL_BUTTON_A;
                    if(event.key.keysym.sym == SDLK_LALT)   mask = CONTROL_BUTTON_B;
                    if(event.key.keysym.sym == SDLK_SPACE)  mask = CONTROL_BUTTON_X;
                    if(event.key.keysym.sym == SDLK_LSHIFT) mask = CONTROL_BUTTON_Y;
                    if(event.key.keysym.sym == SDLK_ESCAPE) mask = CONTROL_BUTTON_SELECT;
                    if(event.key.keysym.sym == SDLK_RETURN) mask = CONTROL_BUTTON_START;
                        
                    if(event.type == SDL_KEYDOWN) 
                    {
                        _control_status[0].status |= mask;
                    }
                    else if(event.type == SDL_KEYUP) 
                    {
                        _control_status[0].status &= (mask ^ 0xFFFFFFFF);
                    }
                }
                
		_control_lock = 0;
	} else {
		_control_lock -= tempDelta;
        }
}

control_state control_check(uint32_t inControl) {
	control_state tempOut;
	if(_control_lock == 0) {
		tempOut.changed = (((_control_status[0].status ^ _control_status[1].status) & inControl) != 0);
		tempOut.pressed = ((_control_status[0].status & inControl) != 0);
	} else {
		tempOut.changed = false;
		tempOut.pressed = false;
	}
	return tempOut;
}

void control_lock(uint32_t inDuration) {
	_control_lock += inDuration;
}
