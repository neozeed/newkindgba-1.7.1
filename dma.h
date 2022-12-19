#include "gba.h"
/******************************************\
* DMA.h by dovoto
\******************************************/

#ifndef DMA_H
#define DMA_H

//these defines let you control individual bit in the control register
// e.g. to copy sprite datta from ROM to VRAM:
/*
// use DMA mode 3
// copy to the next allocated OAM spot in OAM data memory
REG_DMA3SAD = (u32)OAMData[nextOAM];
// copy from our graphics from our sprite array in ROM, copy BAT grafix
REG_DMA3DAD = (u32)creature_gfx[BAT][frame*64];
// copy the first 64 bytes
REG_DMA3CNT = 64 |DMA_16NOW;

// copy the next 64 bytes
REG_DMA3SAD = (u32)OAMData[nextOAM+64];
// 192 is a value needed because of layout of frames in the original gfx
REG_DMA3DAD = (u32)creature_gfx[BAT][192+frame*64]; 
// copy the 64 bytes
REG_DMA3CNT = 64 |DMA_16NOW;

*/


#define DMA_ENABLE 0x80000000
#define DMA_INTERUPT_ENABLE 0x40000000
#define DMA_TIMING_IMMEDIATE 0x00000000
#define DMA_TIMING_VBLANK 0x10000000
#define DMA_TIMING_HBLANK 0x20000000
#define DMA_TIMING_SYNC_TO_DISPLAY 0x30000000
#define DMA_16 0x00000000
#define DMA_32 0x04000000
#define DMA_REPEAT 0x02000000
#define DMA_SOURCE_INCREMENT 0x00000000
#define DMA_SOURCE_DECREMENT 0x00800000
#define DMA_SOURCE_FIXED 0x01000000
#define DMA_DEST_INCREMENT 0x00000000
#define DMA_DEST_DECREMENT 0x00200000
#define DMA_DEST_FIXED 0x00400000
#define DMA_DEST_RELOAD 0x00600000

#define DMA_ENABLE_8BIT 0x8000
#define DMA_INTERUPT_ENABLE_8BIT 0x4000
#define DMA_TIMING_IMMEDIATE_8BIT 0x0000
#define DMA_TIMING_VBLANK_8BIT 0x1000
#define DMA_TIMING_HBLANK_8BIT 0x2000
#define DMA_TIMING_SYNC_TO_DISPLAY_8BIT 0x3000
#define DMA_16_8BIT 0x0000
#define DMA_32_8BIT 0x0400
#define DMA_REPEAT_8BIT 0x0200
#define DMA_SOURCE_INCREMENT_8BIT 0x0000
#define DMA_SOURCE_DECREMENT_8BIT 0x0080
#define DMA_SOURCE_FIXED_8BIT 0x0100
#define DMA_DEST_INCREMENT_8BIT 0x0000
#define DMA_DEST_DECREMENT_8BIT 0x0020
#define DMA_DEST_FIXED_8BIT 0x0040
#define DMA_DEST_RELOAD_8BIT 0x0060

#define DMA_32NOW DMA_ENABLE | DMA_TIMING_IMMEDIATE |DMA_32
#define DMA_16NOW DMA_ENABLE | DMA_TIMING_IMMEDIATE |DMA_16

#endif
