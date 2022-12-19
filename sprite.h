/***************\
* Sprite.h	*
*		*
\***************/

#ifndef SPRITE_H
#define SPRITE_H

//Atribute0 stuff
#define ROTATION_FLAG 		0x100
#define SIZE_DOUBLE			0x200
#define MODE_NORMAL     	0x0
#define MODE_TRANSPARENT	0x400
#define MODE_WINDOWED		0x800
#define MOSAIC				0x1000
#define COLOR_16			0x0000
#define COLOR_256			0x2000
#define SQUARE			0x0         // 00
#define WIDE			0x4000      // 01
#define TALL			0x8000      // 10
// had to swop TALL and WIDE

//Atribute1 stuff
#define ROTDATA(n)		((n) << 9)
#define HORIZONTAL_FLIP		0x1000
#define VERTICAL_FLIP		0x2000
#define SIZE_8			0x0       // 00
#define SIZE_16			0x4000    // 01
#define SIZE_32			0x8000    // 10
#define SIZE_64			0xC000    // 11

/*  SPRITE SIZE TABLE:

             Attrib Attrib   4 bit 
 size         0       1      value
 8  x 8      SQUARE SIZE_8   0000:
 16 x 16     SQUARE SIZE_16  0001:
 32 x 32     SQUARE SIZE_32  0010:
 64 x 64     SQUARE SIZE_64  0011:
 16 x 8      WIDE   SIZE_8   0100:
 32 x 8      WIDE   SIZE_16  0101:
 32 x 16     WIDE   SIZE_32  0110:
 64 x 32     WIDE   SIZE_64  0111:
 8  x 16     TALL   SIZE_8   1000:
 8  x 32     TALL   SIZE_16  1001:
 16 x 32     TALL   SIZE_32  1010:
 32 x 64     TALL   SIZE_64  1011:

*/

// SIZE_16 in WIDE mode is 32*8
// SIZE_8  in WIDE mode is 16*8

//atribute2 stuff

#define PRIORITY(n)		((n) << 10)
#define PALETTE(n)		((n) << 12)


/////structs/////

typedef struct tagOAMEntry
{
	unsigned short int attribute0;
	unsigned short int attribute1;
	unsigned short int attribute2;
	unsigned short int attribute3;
}OAMEntry,*pOAMEntry;

typedef struct tagRotData
{
		
	unsigned short int filler1[3];
	unsigned short int pa;

	unsigned short int filler2[3];
	unsigned short int pb;	
		
	unsigned short int filler3[3];
	unsigned short int pc;	

	unsigned short int filler4[3];
	unsigned short int pd;
}RotData,*pRotData;

extern OAMEntry sprites[128];
extern unsigned short int* OAM;// = (unsigned short int*)0x7000000;
extern pRotData rotData;// = (pRotData)sprites;



#endif
