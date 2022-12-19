#ifndef GFX_H
#define GFX_H

#include "vector.h"
#include "sprite.h"

const static Vertex TopLeft = { 2,2,0,};
const static Vertex DownRight = { 237,106,0,};
extern const unsigned char ZMin; // the value at which the screen sits into the Z plane

#define GFX_X_CENTRE 88
#define GFX_Y_CENTRE 84

//#define CODE_IN_IWRAM __attribute__ ((section (".iwram"), long_call))
// I have to decide whether to place in iwram or not...
//#define CODE_IN_IWRAM  

void gba_triangle_clip(Vertex *pVertices);
void gfx_line_clip(signed long int x1, signed long int y1, signed long int z1,
                   signed long int x2, signed long int y2, signed long int z2);
void gba_circle_clipped(unsigned short int r, signed short centerx, signed short centery, unsigned char filled);
void gba_wobbly_circle_clipped(unsigned short int r, signed short centerx, signed short centery);
void gba_circle(unsigned short int r, signed short centerx, signed short centery, unsigned char filled);

void gba_horizline(unsigned char x, unsigned char y, unsigned char len);
void gba_vertline(unsigned char x, unsigned char y, unsigned char len);

void CopyOAM(unsigned char from, unsigned char to);
void MoveSprite(OAMEntry* sp, signed short int x, signed short int y);
void LoadSprite(unsigned char spriteid, unsigned char copyOAM);
void LoadSpriteData(unsigned char spriteid, unsigned char nFrame);
void LoadOBJPalette(unsigned char pal);

void gfx_draw_menu(unsigned char frame);
void gfx_remove_menu(void);

void gfx_draw_scanner(void);
void gfx_remove_scanner(void);

void display_dial_bar (unsigned char len, unsigned char x, unsigned char y);
void draw_square (unsigned char x, unsigned char y);

void gfx_draw_spot (unsigned char x, unsigned char y);
void gfx_draw_dot (unsigned char x, unsigned char y);
void gfx_draw_big_e(void);
void gfx_remove_big_e(void);
void gfx_draw_big_s(void);
void gfx_remove_big_s(void);
void gfx_draw_big_c(void);
void gfx_remove_big_c(void);

void gfx_clear_status_area(void);
void gfx_draw_selection_square(unsigned char x, unsigned char y);

void print_multi_text(unsigned char x, unsigned char y,const char *txt);

void gfx_centre_text(unsigned char y, char *txt);
void gfx_centre_text_masked(unsigned char y, const char *txt);

void gfx_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void gfx_slow_line(int x1, int y1, int x2, int y2);

void remove_all_scanner_gfx(void);

void load_ship_palette(void);

#endif

