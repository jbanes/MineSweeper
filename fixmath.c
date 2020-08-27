#include "fixmath.h"

uint32_t fix_usmul(uint32_t inArg0, uint32_t inArg1, uint32_t inShift) {
	uint64_t tempResult = ((uint64_t)inArg0 * (uint64_t)inArg1) + (1 << (inShift - 1));
	return (tempResult >> inShift);
}

int32_t fix_smul(int32_t inArg0, int32_t inArg1, uint32_t inShift) {
	int64_t tempResult = ((int64_t)inArg0 * (int64_t)inArg1) + (1 << (inShift - 1));
	return (tempResult >> inShift);
}

uint32_t fix_usdiv(uint32_t inArg0, uint32_t inArg1, uint32_t inShift) {
	return (uint32_t)((((uint64_t)inArg0 << inShift) + (inArg1 >> 1)) / inArg1);
}

int32_t fix_sdiv(int32_t inArg0, int32_t inArg1, uint32_t inShift) {
	return (int32_t)((((int64_t)inArg0 << inShift) + (inArg1 >> 1)) / inArg1);
}
