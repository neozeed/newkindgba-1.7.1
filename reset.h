#ifndef _RESET_H
#define _RESET_H

#define CODE_IN_EWRAM __attribute__ ((section (".ewram"), long_call))

extern void __FarProcedure (void *ptr, ...);
#define RESET_GBA __FarProcedure(reset_gba)

void reset_gba(void) CODE_IN_EWRAM;


#endif

