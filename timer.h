#ifndef __timer_h__
#define __timer_h__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t start;
	uint32_t last;
} timer;

//#define timer_resolution 960000
#define timer_resolution 1

extern timer*   timer_create();
extern void     timer_delete(timer* inTimer);
extern uint32_t timer_delta(timer* inTimer);
extern uint32_t timer_total(timer* inTimer);

#endif
