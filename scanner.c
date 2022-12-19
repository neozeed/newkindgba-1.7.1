// scanner.c
#include "gbalib_asm.h"
// stdlib should include abs, but is broken in devkitadv
#include <stdio.h>
unsigned int abs(int);
#include "space.h"
#include "scanner.h"
#include "shipdata.h"
#include "elite.h"
#include "swat.h"
#include "vector.h"
#include "gfx.h"
#include "main.h"

// scanner center
const static unsigned int scanner_cx = 114;
const static unsigned int scanner_cy = 131;
const static signed int scanner_perspective_dy = -4; // Due to perspective origin is not in centre of ellipse

// max y boundary for long scanner lollipops when x is small and y,z are large.
// This should be larger than scanner height/2 to allow lolps to poke out of 
// the scanner at the top and bottom a small amount. But not too much!
const static unsigned int maxY = 48/2;

const static unsigned char compass_centre_x = 182;
const static unsigned char compass_centre_y = 115;

void draw_lollipop(Object * c);

/*
 * Display the speed bar.
 */
 
void display_speed(void) { 
	
	unsigned char sx,sy;
//	unsigned char i,j;
	unsigned char len;
	unsigned short int colour;

	sx = RIGHT_SIDE_START;
	sy = 110;
	
	// the length should be 0 when speed <= 0
	// up to a max of 32 when speed = max_speed
	if (flight_speed > 0)
		len = flight_speed;
	else
		len = 0;
		
	colour = (flight_speed > gba_div(myship.max_speed << 1 , 3)) ? GFX_COL_DARK_RED : GFX_COL_GOLD;
	gba_setcolour(colour);
	display_dial_bar(len, sx, sy);
//	for (i = 0; i < 4; i++)
//	{
//		for (j = 0; j < len; j++) 
//			gba_drawpixel(sx+j,sy+i);
//	}
//	gba_debug_16_16(0,0,flight_speed); gba_debug_16_16(10,0,len);
}
	

void display_flight_climb(void) {
	unsigned char sx,sy;
	unsigned char i;
	unsigned char pos;
	
	sx = RIGHT_SIDE_START;
	sy = 126;
	
	pos = sx + ((flight_climb * 14) >>3);
	pos += 16;
	gba_setcolour(GFX_COL_LIGHT_BLUE);
	for (i = 0; i < 4; i++)
	{
		// 4 coloured lines in TNK... I'll just do a couple of lines
		gba_drawpixel(pos,sy+i);
		gba_drawpixel(pos+1,sy+i);
	}
}
	
void display_flight_roll(void) { 
	unsigned char sx,sy;
	unsigned char i;
	unsigned char pos;
	
	sx = RIGHT_SIDE_START;
	sy = 118;

	pos = sx - ((flight_roll * 14) >> 5);
	pos += 16;

	for (i = 0; i < 4; i++)
	{
		gba_drawpixel(pos,sy+i);
		gba_drawpixel(pos+1,sy+i);
//		gfx_draw_colour_line (pos + i, sy, pos + i, sy + 7, GFX_COL_GOLD);
	}
//	
}
	
void display_shields(void) { 
	gba_setcolour(GFX_COL_SHIELD_PINK);
	if (front_shield > 7)
		display_dial_bar (1+(front_shield>>3), 10, 109);

	if (aft_shield > 7)
		display_dial_bar (1+(aft_shield>>3), 10, 116);	
}
	

void display_altitude(void) {
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	if (myship.altitude > 3) {
		display_dial_bar (1+(myship.altitude>>3), 10, 143);
	}
	
}
void display_energy(void) {
	signed short int e1,e2,e3,e4;

	e1 = energy > 64 ? 64 : energy;
	e2 = energy > 128 ? 64 : energy - 64;
	e3 = energy > 192 ? 64 : energy - 128;
	e4 = energy - 192;
	gba_setcolour(GFX_COL_SHIELD_PINK);
	if (e4 > 0)
		display_dial_bar (e4>>1, RIGHT_SIDE_START, 135);

	if (e3 > 0)
		display_dial_bar (e3>>1, RIGHT_SIDE_START, 141);

	if (e2 > 0)
		display_dial_bar (e2>>1, RIGHT_SIDE_START, 147);

	if (e1 > 0)
		display_dial_bar (e1>>1, RIGHT_SIDE_START, 153);
}
	
void display_cabin_temp(void) { 
	gba_setcolour(GFX_COL_DARK_RED);
	if (myship.cabtemp > 7)
		display_dial_bar (1+(myship.cabtemp >> 3), 10, 131);
}
	
void display_laser_temp(void) { 
	gba_setcolour(GFX_COL_ORANGE);
	if (laser_temp > 0)
		display_dial_bar (1+(laser_temp >>3), 10, 137);
}
	
void display_fuel(void) {
	gba_setcolour(GFX_COL_BROWN);
	display_dial_bar (gba_div((cmdr->fuel << 5) , 70), 10, 125);
	
}
void display_missiles(void) {
	// display the missiles as coloured squares
	
	// test 
//	unsigned short int missiles = 2;
//	missile_target = MISSILE_ARMED;

	signed char nomiss;
	unsigned short int x,y;

	if (cmdr->missiles == 0)
		return;
	
	nomiss = cmdr->missiles > 4 ? 4 : cmdr->missiles;

	x = (4 - nomiss) * 6 + 19;
	y = 153;
	
	if (missile_target != MISSILE_UNARMED)
	{
		gba_setcolour((missile_target < 0) ? GFX_COL_ORANGE :
											    GFX_COL_DARK_RED);
		draw_square(x, y);
		x += 6;
		nomiss--;
	}
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	for (; nomiss > 0; nomiss--)
	{
		draw_square(x, y);
		x += 6;
	}
	
}


/*
 * Update the scanner and draw all the lollipops.
 * New version based on speccy Elite.
 */

void update_scanner (void)
{
	int i;
	char zoom_str[4];
	
	// Display zoom factor
    if (zoom > 0) {
        siprintf (zoom_str, "%2dx", 1<<zoom);
        gba_setcolour(GFX_COL_DARK_RED);
        gba_print_masked(19, 19, zoom_str);
    }
	
	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if (!((universe[i].type <= 0) ||
			(universe[i].flags & FLG_DEAD) ||
			(universe[i].flags & FLG_CLOAKED)))
		{
		  
		  draw_lollipop(&universe[i]);
		  
		}
		
	}
}

int inRange(Object * c) {
  
  int locx = c->location.x >> 8;
  int locy = c->location.y >> 8;
  int locz = c->location.z >> 8;
  if (abs(locx) > 0x4000 ||
      abs(locy) > 0x4000 ||
      abs(locz) > 0x4000 )
    return 0; //false
  int x = locx >> 8;
  int z = locz >> 8;
  
  int distsq = x*x + z*z;
  
  if (distsq > 0x1600)
    return 0;//false;
  
  // could add a distsq for z and y here?
  return 1;//true; 
}


// draw the lollipop for an object
void draw_lollipop(Object * c) {
  
    long int x1, y1, y2;
  
//  if (inRange(c)) {
    unsigned short int colour;

// trimmed the following code for speed reasons
//    int locx = c->location.x >> 8;
//    int locy = c->location.y >> 8;
//    int locz = c->location.z >> 8;
//    
////    int x = 3 * (locx) >> (10-zoom);
////    int y = 5 * (locy) >> (12-zoom);
////    int z = 5 * (locz) >> (12-zoom);
//
//    int x = (locx) >> (8-zoom);
//    int y = (locy) >> (10-zoom);
//    int z = (locz) >> (10-zoom);

    long int x = c->location.x >> (17-zoom);
    long int y = c->location.y >> (19-zoom);
    long int z = c->location.z >> (19-zoom);

    if (abs(z) <= 40) { // region where approximation OK
        // Perspective shrinking (simplified, approximate formulas, not exact)
        x = (-4 * z + 357) * x / 441;
        y = (-4 * z + 357) * y / 441;
        z = (-4 * z + 357) * z / 441;
    }

    x1 = scanner_cx + x;
    y1 = scanner_cy + scanner_perspective_dy - z; // base
    y2 = y1 - y;       // head
    
    if (abs(x) > SCANNER_MAX_X) 
        return;
    else if ((abs(y1-scanner_cy) > scanner_rim_y[abs(x1-scanner_cx)]))
        return;
    
    // trim stalk when y and z are big and x is small
    long int top = y1;  
    long int bottom = y2; 
    int head = 1; // head is at bottom
    if (bottom > top) {
      long int tmp = top;
      top = bottom;
      bottom = tmp;
      head = 0; // head is at top
    }
    
    int drawhead = 1;
    if (top > (long int)(scanner_cy+maxY)) {
      if (head == 0) drawhead = 0;
      top = scanner_cy+maxY;
    }
    if (bottom < (long int)(scanner_cy-maxY)) {
      if (head == 1) drawhead = 0;
      bottom = scanner_cy-maxY;
    }
    
    if (cmdr->equipment & ID_UNIT) {
        colour = (c->flags & FLG_HOSTILE) ? GFX_COL_GOLD : GFX_COL_WHITE;
      	switch (c->type)
      	{
            case SHIP_MISSILE:
            	colour = GFX_COL_PIN_PINK;
            	break;
            
            case SHIP_DODEC:
            case SHIP_CORIOLIS:
            	colour = GFX_COL_LIGHT_GREEN;
            	break;
            	
            case SHIP_VIPER:
            	colour = GFX_COL_MED_BLUE;
            	break;
      	  
            case SHIP_ESCAPE_CAPSULE:
            case SHIP_ALLOY:
            case SHIP_CARGO:
            	colour = GFX_COL_LIGHT_BLUE;
            	break;
            	
            case SHIP_ASTEROID:
            case SHIP_BOULDER:
            case SHIP_ROCK:
            case SHIP_HERMIT:
            colour = GFX_COL_GREY;
      	}
    }
    else
        colour = GFX_COL_WHITE;
  	
  	gba_setcolour(colour);
  	// this is the head	
        if (drawhead) {
          gba_drawpixel(x1-2, y2);gba_drawpixel(x1-2, y2-1);    
          gba_drawpixel(x1-1, y2);gba_drawpixel(x1-1, y2-1);
          gba_drawpixel(x1  , y2);gba_drawpixel(x1  , y2-1);  
          gba_drawpixel(x1+1, y2);gba_drawpixel(x1+1, y2-1);
        }
  	
  	// this is the tail, draw a line from x1,y1 to x1,y2
  	
    if (x1 & 1) {
  	  // uneven byte
  	  gfx_slow_line(x1,   top, x1,   bottom);
  	  gfx_slow_line(x1+1,   top, x1+1,   bottom);
  	} else {
  	  gba_drawline(x1,   159-top, x1,   159-bottom);
  	}
//  }  // if inRange(c)
	
	
}

// new compass code based roughly on speccy version
void doCompass(Object* c) {

  int distance;
  int x, y, z;

  x = c->location.x >> 8;
  y = c->location.y >> 8;
  z = c->location.z >> 8;

  // check overflow of s32
  int overflow = (abs(x) > 0xb504 ||
      abs(y) > 0xb504 ||
      abs(z) > 0xb504); /*overflow guaranteed: b505*b505 > 0x80000000 */
  if (!overflow) {
    distance = x*x + y*y;
    overflow = (distance < 0);
    if (!overflow) {
      distance += z*z;
      overflow = (distance < 0);
    }

  }

  if (overflow) {
    // use obj far away code
    distance = ((x >> 8)*(x >> 8)) +
               ((y >> 8)*(y >> 8))+
               ((z >> 8)*(z >> 8));

  } else {
    // use close code
    // distance = x*x + y*y +z*z; /*already is this*/
    x <<= 8;
    y <<= 8;

  }
  // find projected position:
  distance = gba_sqrt(distance);
  if (distance == 0)
    distance = 0xFFFF;
  int px = gba_div(x , distance);
  int py = gba_div(y , distance);

  //cx, cy rng 0-7
  int cx = ((px >> 4) + 1) >> 1;
  int cy = ((py >> 4) + 1) >> 1;
  // round properly

// max range check not needed
//      if (abs(cx) > 7)
//        cx = cx<0?-7:7;
//      if (abs(cy) > 7)
//        cy = cy<0?-7:7;

  if (z < 0)
	{
		gba_setcolour(GFX_COL_DARK_RED);
	}
	else
	{
		gba_setcolour(GFX_COL_LIGHT_GREEN);
	}
	gfx_draw_spot(compass_centre_x + cx, compass_centre_y-cy);


}

void update_compass(void) {
  unsigned char un = 0;

	if (game_state_flag & WITCHSPACE)
		return;
	
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
		un = 1;
  doCompass(&universe[un]);
}


