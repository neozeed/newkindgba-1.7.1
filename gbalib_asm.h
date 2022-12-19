// (c) Peter Horsman, 2002
// gba library header file.
// Pete ( dooby@bits.bris.ac.uk / http://bits.bris.ac.uk/dooby/ )

// Much of the information has been learnt from other places.
// A big thank you to those who traced, disassembled or guessed.

#ifndef __gba_h
#define __gba_h
#include "gba.h"

#define CODE_IN_IWRAM __attribute__ ((section (".iwram"), long_call))
#define CODE_IN_ROM __attribute__ ((section (".text"), long_call))
#define IN_IWRAM __attribute__ ((section (".iwram")))
#define IN_EWRAM __attribute__ ((section (".ewram")))

#define MULTIBOOT volatile const u8 __gba_multiboot;

#define START_TIMER0 { \
  *(volatile u16 *)0x04000100 = 0; \
  *(volatile u16 *)0x04000102 = 0x0080; \
}

#define STOP_TIMER0(x) { \
  (x) = *(volatile u16 *)0x04000100; \
  *(volatile u16 *)0x04000102 = 0; \
}

#ifndef word
#define word unsigned int
#endif

#ifndef byte
#define byte unsigned char
#endif

#ifndef half
#define half unsigned short int
#endif

#define XOFF	120
#define YOFF	80

#ifndef TRUE
#define TRUE (0==0)
#endif

#ifndef FALSE
#define FALSE (0==1)
#endif

// Joypad bit mask.
#define PAD_A		0x0001
#define PAD_B		0x0002
#define PAD_SELECT	0x0004
#define PAD_START	0x0008
#define PAD_RIGHT	0x0010
#define PAD_LEFT	0x0020
#define PAD_UP		0x0040
#define PAD_DOWN	0x0080
#define PAD_R		0x0100
#define PAD_L		0x0200

// Returns a bitmask of the currently pressed buttons in the
// bottom half of the word.
unsigned short gba_readpad(void);

// Hopefully this won't happen, but it's nice to trap errors.
void gba_die(void);

// Library initialisation.
void gba_init(void);
void gba_reset(void);

// Bank stuff.
void gba_initbank(void);
void gba_swapbank(void);
void gba_setdbank(byte n);
void gba_setwbank(byte b);
void gba_vsync(void);

// Screen mode handling.
void gba_setmode(word md);
word gba_getmode(void);

// Screen clearing.
void gba_clsUnroll(void);
void gba_clsUnroll2(u32 colour) CODE_IN_IWRAM;
void gba_cls(void);
void gba_clsW(void);		// Blits 4 words @ a time - should be faster, theoretically...
void gba_clg(word col);

// Maths stuff.
word gba_umod(word numerator, word denominator);
signed long int gba_div(signed long int numerator, signed long int denominator) CODE_IN_IWRAM;
word gba_qdiv(word denominator);
word gba_udiv(word numerator, word denominator);
word gba_rand(void);
signed long int gba_sin(int ang);		// Check range for 0-359.
signed long int gba_cos(int ang);		// Check range.
signed long int gba_sinq(int ang) CODE_IN_IWRAM;		// No range check.
signed long int gba_cosq(int ang) CODE_IN_IWRAM;		// No range check.
signed long int gba_sqrt(signed long int n);
signed long int gba_atan(signed long int n);

// Plot string to character-aligned positions.
void gba_print(int x, int y, const char *msg);
void gba_print_masked(int x, int y, char *msg);
// Debug 'N' bits in base 'M'.
void gba_debug_32_16(int x, int y, unsigned long num);
void gba_debug_16_16(int x, int y, unsigned short num);
void gba_debug_16_2(int x, int y, unsigned short num);
void gba_debug_masked(int x, int y, word num);

// Various drawing primitives.
void gba_scanline(u32 x1, u32 y1, u32 x2);
void gba_drawline(u32 x1, u32 y1, u32 x2, u32 y2);
void gba_drawpixel(u32 x, u32 y);
void gba_setpixel(u32 x, u32 y, int c);
void gba_triangle(u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3) CODE_IN_IWRAM;
void gba_triangleF(u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3);
void gba_triangleQ(u32 x1, u32 y1, u32 x2, u32 y2, u32 x3, u32 y3);

// Colour handling.
void gba_setcolour(int c) CODE_IN_IWRAM;
void gba_setpalette(int c, int r, int g, int b) CODE_IN_IWRAM;

/* To do. */
/* set colour to R, G, B (each 0-255) - only use >256 cols. */
//void vga_setrgbcolor(int r, int g, int b);
/* Returns pixel value. */
//int vga_getpixel(int x, int y);
/* Set the display start address offset. */
//void vga_setdisplaystart(int a);
/* Draw a horizontal line of pixels using a colour table. */
//int vga_drawscanline(int line, unsigned char *color);
//int vga_drawscansegment(unsigned char *colors, int x, int y, int length);

#endif // __gba_h
