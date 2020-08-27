#ifndef __fixmath_h__
#define __fixmath_h__

#include <stdint.h>

extern uint32_t fix_usmul(uint32_t inArg0, uint32_t inArg1, uint32_t inShift);
extern int32_t  fix_smul(int32_t inArg0, int32_t inArg1, uint32_t inShift);
extern uint32_t fix_usdiv(uint32_t inArg0, uint32_t inArg1, uint32_t inShift);
extern int32_t  fix_sdiv(int32_t inArg0, int32_t inArg1, uint32_t inShift);

#endif
