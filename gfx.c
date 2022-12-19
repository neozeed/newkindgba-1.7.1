#include <malloc.h>
#include <string.h>
// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include "gfx.h"
#include "dma.h"
#include "elite.h"
#include "shipdata.h"
#include "gbalib_asm.h"
#include "random.h"
#include "unpacklz77.h"
#include "elite_sprites.h"

#include "registers.h"
// added before first compilation succeeded, TM

OAMEntry sprites[128];
unsigned short int* OAM = (unsigned short int*)0x7000000;
pRotData rotData = (pRotData)sprites;
const unsigned char ZMin = 1;
void SetSpritePriority(unsigned char spriteid, unsigned char priority);
void CLIP_Front(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P/*, unsigned char nVal*/);
void CLIP_Z(Vertex *pVertices, Vertex *pZClippedVertices, unsigned char * nOutPoints);

void CLIP_Left(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P);
void CLIP_Right(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P);
void CLIP_Top(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P);
void CLIP_Bottom(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P);

void CLIP_Polygon(Vertex *pVertices, Vertex *pClippedVertices, signed char * nOutPoints); 

void symmetricPlot (signed short int x, signed short int y, signed short centerx, signed short centery, unsigned char filled);
void symmetricPlot_clipped (signed short int x, signed short int y, signed short centerx, signed short centery, unsigned char filled);
void symmetricPlot_wobbly_clipped (signed short int x, signed short int y, signed short centerx, signed short centery, signed short radius);
void draw_clipped_scan_lines(signed short int x1, signed short int y1,
                             signed short int x2, signed short int y2,
                             signed short int x3, signed short int y3,
                             signed short int x4, signed short int y4);
void gfx_slow_line(int x1, int y1, int x2, int y2);

extern unsigned char _binary_master_pal_start[];
extern unsigned char _binary_master2_pal_start[];

void gba_circle_clipped(unsigned short int r, signed short centerx, signed short centery, unsigned char filled) {
  signed long int x = 0;
  signed long int y = r;
  signed long int g = 3 - 2*r;
  signed long int diagonalInc = 10 - 4*r;
  unsigned short int rightInc = 6;
  while (x <= y) {
    symmetricPlot_clipped (x, y, centerx, centery, filled);
    
    if (g >=  0) {
      g += diagonalInc;
      diagonalInc += 8;
      y -= 1;
    }
    else {
      g += rightInc;
      diagonalInc += 4;
    }
    rightInc += 4;
    x += 1;
  }
}

void gba_wobbly_circle_clipped(unsigned short int r, signed short centerx, signed short centery) {
  signed long int x = 0;
  signed long int y = r;
  signed long int g = 3 - 2*r;
  signed long int diagonalInc = 10 - 4*r;
  unsigned short int rightInc = 6;
  while (x <= y) {
    symmetricPlot_wobbly_clipped (x, y, centerx, centery, r);
    
    if (g >=  0) {
      g += diagonalInc;
      diagonalInc += 8;
      y -= 1;
    }
    else {
      g += rightInc;
      diagonalInc += 4;
    }
    rightInc += 4;
    x += 1;
  }
}

void gba_circle(unsigned short int r, signed short centerx, signed short centery, unsigned char filled) {
  signed long int x = 0;
  signed long int y = r;
  signed long int g = 3 - 2*r;
  signed long int diagonalInc = 10 - 4*r;
  unsigned short int rightInc = 6;
  while (x <= y) {
    symmetricPlot (x, y, centerx, centery, filled);
    
    if (g >=  0) {
      g += diagonalInc;
      diagonalInc += 8;
      y -= 1;
    }
    else {
      g += rightInc;
      diagonalInc += 4;
    }
    rightInc += 4;
    x += 1;
  }
}


void symmetricPlot_clipped (signed short int x, signed short int y, signed short centerx, signed short centery, unsigned char filled) {
  // calculate the 4 x and y values as they are used > 1 time each
  signed short int x1 = centerx - abs(x);
  signed short int x2 = centerx + abs(x);
  signed short int x3 = centerx - abs(y);
  signed short int x4 = centerx + abs(y);
  
  // when bug occurs... x3 = -128, x4 = 4
  
  signed short int y1 = centery - abs(y);
  signed short int y2 = centery + abs(y);
  signed short int y3 = centery - abs(x);
  signed short int y4 = centery + abs(x);
  
  // unsigned short int i; // loop val
  if (filled) {
    // draw the scan lines...
    draw_clipped_scan_lines(x1, y1, x2, y2, x3, y3, x4, y4);
  }
  else {
	// draw an empty circle
    if (x2 > TopLeft.x &&  y2 > TopLeft.y && x2 < DownRight.x && y2 < DownRight.y) 
      gba_drawpixel(x2, y2);
    if (x2 > TopLeft.x &&  y1 > TopLeft.y && x2 < DownRight.x && y1 < DownRight.y) 
      gba_drawpixel(x2, y1);
    if (x1 > TopLeft.x &&  y2 > TopLeft.y && x1 < DownRight.x && y2 < DownRight.y) 
      gba_drawpixel(x1, y2);
    if (x1 > TopLeft.x &&  y1 > TopLeft.y && x1 < DownRight.x && y1 < DownRight.y) 
      gba_drawpixel(x1, y1);
    if (x3 > TopLeft.x &&  y4 > TopLeft.y && x3 < DownRight.x && y4 < DownRight.y) 
      gba_drawpixel(x3, y4);
    if (x3 > TopLeft.x &&  y3 > TopLeft.y && x3 < DownRight.x && y3 < DownRight.y) 
      gba_drawpixel(x3, y3);
    if (x4 > TopLeft.x &&  y4 > TopLeft.y && x4 < DownRight.x && y4 < DownRight.y) 
      gba_drawpixel(x4, y4);
    if (x2 > TopLeft.x &&  y3 > TopLeft.y && x2 < DownRight.x && y3 < DownRight.y) 
      gba_drawpixel(x4, y3);
  }
}

void draw_clipped_scan_lines(signed short int x1, signed short int y1,
                             signed short int x2, signed short int y2,
                             signed short int x3, signed short int y3,
                             signed short int x4, signed short int y4) {

    signed short int slx1, slx2;
    
    if (x1 >= DownRight.x)
        ; // nothing to plot since DownRight.x < x1 < x2 
    else if (x2 <= TopLeft.x)
        ; // nothing to plot since x1 < x2 < TopLeft.x
    else {
        if (x1 <= TopLeft.x)
            x1 = TopLeft.x+1; // x1 <= TopLeft.x < x2
        if (x2 >= DownRight.x)
            x2 = DownRight.x-1; // x1 < DownRight.x <= x2

        slx1 = (x1+1)&~1; // ((x1+1)>>1)<<1;
        slx2 = (x2-1)&~1; // ((x2-1)>>1)<<1;

        if (y1 > TopLeft.y && y1 < DownRight.y) {
            if (slx2 >= slx1)
                gba_scanline(slx1, 159-y1, slx2);
            gba_drawpixel(x1, y1);
        	gba_drawpixel(x2, y1);
        }                

        if (y2 > TopLeft.y && y2 < DownRight.y) {
            if (slx2 >= slx1) 
                gba_scanline(slx1, 159-y2, slx2);
            gba_drawpixel(x1, y2);
        	gba_drawpixel(x2, y2);
        }
        
    } 
    
    if (x3 >= DownRight.x)
        ; // nothing to plot since DownRight.x < x3 < x4 
    else if (x4 <= TopLeft.x)
        ; // nothing to plot since x3 < x4 < TopLeft.x
    else {
        if (x3 <= TopLeft.x)
            x3 = TopLeft.x+1; // x3 <= TopLeft.x < x4
        if (x4 >= DownRight.x)
            x4 = DownRight.x-1; // x3 < DownRight.x <= x4

        slx1 = (x3+1)&~1; // ((x3+1)>>1)<<1;
        slx2 = (x4-1)&~1; // ((x4-1)>>1)<<1;
        
        if (y3 > TopLeft.y && y3 < DownRight.y) {
            if (slx2 >= slx1) 
                gba_scanline(slx1, 159-y3, slx2);
            gba_drawpixel(x3, y3);
        	gba_drawpixel(x4, y3);
        }
                
        if (y4 > TopLeft.y && y4 < DownRight.y) {
            if (slx2 >= slx1) 
                gba_scanline(slx1, 159-y4, slx2);
            gba_drawpixel(x3, y4);
        	gba_drawpixel(x4, y4);
        }
    } 
    
}

void symmetricPlot_wobbly_clipped (signed short int x, signed short int y, signed short centerx, signed short centery, signed short radius) {
  signed short int tmp;
  // calculate the 4 x and y values as they are used > 1 time each
  
//  signed short int sx = -((radius * (2 + (GetRand(65535) & 7))) >> 8);
//  signed short int ex = (radius * (2 + (GetRand(65535) & 7))) >> 8;

    signed short int x1 = centerx - x + ((radius * (2 + (GetRand(65535) & 7))) >> 7);
    signed short int x2 = centerx + x - ((radius * (2 + (GetRand(65535) & 7))) >> 7);
    signed short int x3 = centerx - y + ((radius * (2 + (GetRand(65535) & 7))) >> 7);
    signed short int x4 = centerx + y - ((radius * (2 + (GetRand(65535) & 7))) >> 7);
    
    signed short int y1 = centery - abs(y);
    signed short int y2 = centery + abs(y);
    signed short int y3 = centery - abs(x);
    signed short int y4 = centery + abs(x);
    
    if (x2 < x1)  {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (x4 < x3)  {
        tmp = x4;
        x4 = x3;
        x3 = tmp;
    }

    // draw the scan lines...
    draw_clipped_scan_lines(x1, y1, x2, y2, x3, y3, x4, y4);
}

// do not check the size of this circle
void symmetricPlot (signed short int x, signed short int y, signed short centerx, signed short centery, unsigned char filled) {
  signed short int slx1, slx2;
  // calculate the 4 x and y values as they are used > 1 time each
  signed short int x1 = centerx - abs(x);
  signed short int x2 = centerx + abs(x);
  signed short int x3 = centerx - abs(y);
  signed short int x4 = centerx + abs(y);
  
  signed short int y1 = centery - abs(y);
  signed short int y2 = centery + abs(y);
  signed short int y3 = centery - abs(x);
  signed short int y4 = centery + abs(x);
  
  // unsigned short int i; // loop val
  if (filled) {
    // draw the scan lines...

//    if (x2 < x1)  {
//      tmp = x1;
//      x1 = x2;
//      x2 = tmp;
//    }

    slx1 = (x1+1)&~1; // (x1+1>>1)<<1;
    slx2 = (x2-1)&~1; // (x2-1>>1)<<1;
        
    if (x2 > x1) {
        gba_scanline(slx1, 159-y2, slx2);
        gba_scanline(slx1, 159-y1, slx2);
    }
        
	gba_drawpixel(x1, y2);
	gba_drawpixel(x2, y2);
  
    gba_drawpixel(x1, y1);
	gba_drawpixel(x2, y1);

//    if (x4 < x3)  {
//      tmp = x4;
//      x4 = x3;
//      x3 = tmp;
//    }

    slx1 = (x3+1)&~1; // (x3+1>>1)<<1;
    slx2 = (x4-1)&~1; // (x4-1>>1)<<1;

    if (x4 > x3) {
        gba_scanline(slx1, 159-y4, slx2);
        gba_scanline(slx1, 159-y3, slx2);
    }
	
	gba_drawpixel(x3, y4);
	gba_drawpixel(x4, y4);
    
	gba_drawpixel(x3, y3);
	gba_drawpixel(x4, y3);
  }
  else {
		// draw an empty circle
    gba_drawpixel(x2, y2);
    gba_drawpixel(x2, y1);
    gba_drawpixel(x1, y2);
    gba_drawpixel(x1, y1);
    gba_drawpixel(x3, y4);
    gba_drawpixel(x3, y3);
    gba_drawpixel(x4, y4);
    gba_drawpixel(x4, y3);
  }
}


void gba_horizline(unsigned char x, unsigned char y, unsigned char len){
  unsigned char i;
  for (i = 0; i < len; i++) {
    gba_drawpixel(x+i,y);
  }
}

void gba_vertline(unsigned char x, unsigned char y, unsigned char len){
  unsigned char i;
  for (i = 0; i < len; i++) {
    gba_drawpixel(x,y+i);
  }
}


void gba_triangle_clip(Vertex *pVertices) {  
  // from a face to triangles on screen
  // clip the face to Z plane (front of the view volume) and the screen edges
  Vertex  pZClippedVertices[4]; // at most 4 points for when the triangle does this sort of thing
  /*
      _______
      \     |
...... \____|............ z plane
       
        
  */
  // 7 points is the maximum you can get from 1 triangle clipped to the screen
  // that is when all 3 points are outside and 1 point is outside a corner
  Vertex  pClippedVertices[7];
  
  signed char i; // general looping val..
  signed char nOutPoints = 0;
  
  CLIP_Z(pVertices,  pZClippedVertices, &nOutPoints);
  // after clipping to the Z face, there will be at most 2 triangles.  
  // clip each of the 2 triangles to the screen. This will produce more triangles...
  if (nOutPoints == 3) {
    // clip just 1 triangle, this is easy...
    CLIP_Polygon(pZClippedVertices,  pClippedVertices, &nOutPoints);
    // the pClippedVertices array contains the points... now plot them!
    // easiest way is in a loop
    nOutPoints -= 2;
    if (nOutPoints > 0) {
//      gba_setcolour(GFX_COL_WHITE);
      for (i = 0; i < nOutPoints; i++) {
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[2+i].x, pClippedVertices[2+i].y);
//        gfx_line(pClippedVertices[2+i].x,pClippedVertices[2+i].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
        gba_triangle(pClippedVertices[0].x,pClippedVertices[0].y,
          pClippedVertices[1+i].x, pClippedVertices[1+i].y, 
          pClippedVertices[2+i].x, pClippedVertices[2+i].y );
      }
    }
  } else if (nOutPoints == 4) {
    // do the first triangle
    
    CLIP_Polygon(pZClippedVertices, &pClippedVertices[0], &nOutPoints);
    // the pClippedVertices array contains the points... now plot them
    nOutPoints -= 2;
    if (nOutPoints > 0) {
      // check that this triangle is on screen (it might not be )
//      nOutPoints -= 2;
//gba_setcolour(GFX_COL_WHITE);
      for (i = 0; i < nOutPoints; i++) {
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[2+i].x, pClippedVertices[2+i].y);
//        gfx_line(pClippedVertices[2+i].x,pClippedVertices[2+i].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
				gba_triangle(pClippedVertices[0].x,pClippedVertices[0].y,
				  pClippedVertices[i+1].x, pClippedVertices[i+1].y, 
				  pClippedVertices[i+2].x, pClippedVertices[i+2].y );
      }
    }
    
    // do the second triangle
    Vertex  triangle2[3];
    triangle2[0] = pZClippedVertices[0];
    triangle2[1] = pZClippedVertices[2];
    triangle2[2] = pZClippedVertices[3];
    CLIP_Polygon(triangle2,  pClippedVertices, &nOutPoints);

    nOutPoints -= 2;
    if (nOutPoints > 0) {
//      nOutPoints -= 2;
//gba_setcolour(GFX_COL_WHITE);
      for (i = 0; i < nOutPoints; i++) {
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
//        gfx_line(pClippedVertices[0].x,pClippedVertices[0].y,
//          pClippedVertices[2+i].x, pClippedVertices[2+i].y);
//        gfx_line(pClippedVertices[2+i].x,pClippedVertices[2+i].y,
//          pClippedVertices[1+i].x, pClippedVertices[1+i].y);
        gba_triangle(pClippedVertices[0].x,pClippedVertices[0].y,
            pClippedVertices[i+1].x, pClippedVertices[i+1].y, 
            pClippedVertices[i+2].x, pClippedVertices[i+2].y );
      }
    }
  }
  
}


void CLIP_Front(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P/*, unsigned char nVal*/) {
  
  // check if the edge is either totally inside the viewport, leaving it or entering it
  unsigned char nCount = 0;
  // If it's inside, store P
  if ( (S.z >= ZMin) && (P.z >= ZMin) ) {
    pVertices[nCount++] = P;
  }
  
  // If it's leaving the viewport, i.e. S inside P outside, 
  // calculate and store i (the intersection with the view port)
  if ( (S.z>=ZMin) && (P.z < ZMin) )
  {
    if ((P.z-S.z)!= 0) {
      pVertices[nCount  ].x = S.x + gba_div(((P.x-S.x)*(ZMin-S.z)),(P.z-S.z));
      pVertices[nCount  ].y = S.y + gba_div(((P.y-S.y)*(ZMin-S.z)),(P.z-S.z));
    } else {
      pVertices[nCount  ].x = S.x + ((P.x-S.x)*(ZMin-S.z));
      pVertices[nCount  ].y = S.y + ((P.y-S.y)*(ZMin-S.z));
    }
    pVertices[nCount++].z = ZMin;
//    gba_print(0, 11+nVal, "IO ");
  }
  
  // If it's entering the viewport. i.e. S is outside but P inside
  // calc and store i and P
  if ( (S.z<ZMin) && (P.z>=ZMin) )
  {
    if ((P.z-S.z) != 0) {
      pVertices[nCount  ].x = S.x + gba_div(((P.x-S.x)*(ZMin-S.z)),(P.z-S.z));
      pVertices[nCount  ].y = S.y + gba_div(((P.y-S.y)*(ZMin-S.z)),(P.z-S.z));
    } else {
      pVertices[nCount  ].x = S.x + ((P.x-S.x)*(ZMin-S.z));
      pVertices[nCount  ].y = S.y + ((P.y-S.y)*(ZMin-S.z));
    }
    
    pVertices[nCount++].z = ZMin;
    pVertices[nCount++]   = P;
  }
  
  *oCount = nCount;
}

// clip a Polygon to the Z plane...
// the vertices coming in are stored in pVertices
// the vertices going out are stored in pZClippedVertices, which should already have been 
// initialised with memory
void CLIP_Z(Vertex *pVertices, Vertex *pZClippedVertices, unsigned char * nOutPoints)
{
  unsigned char nInPoints = 3;
   unsigned char d,v;
   unsigned char nPoints = 0;
//   unsigned char nCounter = 0;
   for (v=0; v < nInPoints; v++)
   {
      d=v+1;
      unsigned char nTmpPoints = 0;
      if(d == nInPoints)
        d=0;
      
      CLIP_Front(&pZClippedVertices[nPoints], &nTmpPoints, pVertices[v],pVertices[d]);
      // increase the index into the ZClipped array
      nPoints+=nTmpPoints;
   }
   // set the number of points in the Z clipped triangle
   *nOutPoints = nPoints;
}

// clip a polygon (well, triangle) to the top bottom left and right of the screen
void CLIP_Polygon(Vertex *pVertices, Vertex *pClippedVertices, signed char * nOutPoints)
{
  
  unsigned char nInPoints = 3;
  unsigned char d,v;
  unsigned char nPoints = 0;
  unsigned char nCurrentPoints = 0;
  unsigned char nTmpPoints = 0;
  Vertex  pTmpVertices[7]; // the temp vertices...

  // nCurrentPoints stores how many vertices we currently have stored
  // the max is 7 for a triangle clipped to all 4 view vertices.
  nCurrentPoints = nInPoints;
  // clip all the points in our original polygon to the left of the screen
  d = 0;
  for (v=0; v < nCurrentPoints; v++)  {
    d=v+1;
    nTmpPoints = 0;
    if(d == nCurrentPoints)
      d=0;
    CLIP_Left(&pTmpVertices[nPoints], &nTmpPoints,pVertices[v],pVertices[d]);
    // increase the index into the Clipped array
    nPoints+=nTmpPoints;
  }

  // stored in pTmpVertices are the original vertices clipped to the left of the screen.
  // clip these to the right of the screen and store the temp verts in pClippedVertices
  nCurrentPoints = nPoints;
  nPoints = 0;

  d = 0;
  if (nCurrentPoints == 0) {
    *nOutPoints = nCurrentPoints;
    return;
  }
  
  for (v=0; v<nCurrentPoints; v++)
  {
    d=v+1;
    nTmpPoints = 0;
    if(d == nCurrentPoints)
      d=0;
    CLIP_Right(&pClippedVertices[nPoints], &nTmpPoints,pTmpVertices[v],pTmpVertices[d]);
    // increase the index into the Clipped array
    nPoints+=nTmpPoints;
  }

  // now in pClippedVertices we have the original Polygon clipped to both the left and right of the screen
  // need to clip this to the top of the screen. Store the temp val in pTmpVertices
  nCurrentPoints = nPoints;
  nPoints = 0;

  d = 0;
  if (nCurrentPoints == 0) {
    *nOutPoints = nCurrentPoints;
    return;
  }
  for (v=0; v<nCurrentPoints; v++)
  {
    d=v+1;
    nTmpPoints = 0;
    if(d == nCurrentPoints)
      d=0;
    CLIP_Top(&pTmpVertices[nPoints], &nTmpPoints,pClippedVertices[v],pClippedVertices[d]);
    // increase the index into the Clipped array
    nPoints+=nTmpPoints;
  }

  // in TmpPoly we have the original Polygon clipped to the left, right and top of the screen
  // clip to the bottom of the screen storing the vertices in pClippedVertices, then we are done!
  nCurrentPoints = nPoints;
  nPoints = 0;

  d = 0;
  if (nCurrentPoints == 0) {
    *nOutPoints = nCurrentPoints;
    return;
  }
  for (v=0; v<nCurrentPoints; v++)
  {
    d=v+1;
    nTmpPoints = 0;
    if(d == nCurrentPoints)
      d=0;
    CLIP_Bottom(&pClippedVertices[nPoints], &nTmpPoints,pTmpVertices[v],pTmpVertices[d]);
    // increase the index into the Clipped array
    nPoints+=nTmpPoints;
  } 
  nCurrentPoints = nPoints;
  nPoints = 0;
  
  *nOutPoints = nCurrentPoints;
  
}
  



void CLIP_Left(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P)
{

  unsigned char nCount = 0;
  // m = dy/dx
  // dx = P.x - S.x dx = P.y - S.y
  // ************OK************
  if ( (S.x>=TopLeft.x) && (P.x>=TopLeft.x) ) {
    pVertices[nCount].x=P.x;
    pVertices[nCount++].y=P.y;

  }
  
  // *********LEAVING**********
  if ( (S.x>=TopLeft.x) && (P.x<TopLeft.x) ) {
    pVertices[nCount].x=TopLeft.x;
    if ((P.x-S.x) != 0)
      pVertices[nCount++].y=S.y+ gba_div(((P.y-S.y)*(TopLeft.x-S.x)),(P.x-S.x));
    else  
      pVertices[nCount++].y=S.y+ ((P.y-S.y)*(TopLeft.x-S.x));

  }
  // ********ENTERING*********
  if ( (S.x<TopLeft.x) && (P.x>=TopLeft.x) )  {
    pVertices[nCount].x=TopLeft.x;
    if ((P.x-S.x) != 0)
      pVertices[nCount++].y=S.y+ gba_div(((P.y-S.y)*(TopLeft.x-S.x)),(P.x-S.x));
    else
      pVertices[nCount++].y=S.y+ ((P.y-S.y)*(TopLeft.x-S.x));
    pVertices[nCount].x=P.x;
    pVertices[nCount++].y=P.y;

  }
  *oCount = nCount;
}

void CLIP_Right(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P/*, unsigned char nVal*/) {

   unsigned char nCount = 0;
   signed long dx,dy;
   dx=P.x-S.x;  dy=P.y-S.y;
   // ************OK************
   if ( (S.x<=DownRight.x) && (P.x<=DownRight.x) ) {
      pVertices[nCount].x=P.x;
      pVertices[nCount++].y=P.y;
   }
    
   // *********LEAVING**********
   if ( (S.x<=DownRight.x) && (P.x>DownRight.x) )
   {
      pVertices[nCount].x=DownRight.x;
      if (dx!= 0)
        pVertices[nCount++].y=S.y+gba_div((dy*(DownRight.x-S.x)),dx);
      else
        pVertices[nCount++].y=S.y+(dy*(DownRight.x-S.x));
   }
   // ********ENTERING*********
   if ( (S.x>DownRight.x) && (P.x<=DownRight.x) )
   {
      pVertices[nCount].x=DownRight.x;
      if (dx != 0)
        pVertices[nCount++].y=S.y+gba_div((dy*(DownRight.x-S.x)),dx);
      else
        pVertices[nCount++].y=S.y+(dy*(DownRight.x-S.x));
      pVertices[nCount].x=P.x;
      pVertices[nCount++].y=P.y;
   }
   *oCount = nCount;
}

void CLIP_Top(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P/*, unsigned char nVal*/) {
   unsigned char nCount = 0;
   signed long dx,dy;
   dx=P.x-S.x;  dy=P.y-S.y;
   // m = dy/dx -> /m = dx/dy
   // ************OK************
   if ( (S.y>=TopLeft.y) && (P.y>=TopLeft.y) ) {
    pVertices[nCount].x=P.x;
    pVertices[nCount++].y=P.y;
   }
    
   // *********LEAVING**********
   if ( (S.y>=TopLeft.y) && (P.y<TopLeft.y) )
   {
      if(dy)
         pVertices[nCount].x=S.x+gba_div(dx*(TopLeft.y-S.y),dy);
      else {
         pVertices[nCount].x=S.x + dx*(TopLeft.y-S.y);
        }
        
      pVertices[nCount++].y=TopLeft.y;
   }
   // ********ENTERING*********
   if ( (S.y<TopLeft.y) && (P.y>=TopLeft.y) )
   {
      if(dy)
         pVertices[nCount].x=S.x+gba_div(dx*(TopLeft.y-S.y),dy);
      else {
         pVertices[nCount].x=S.x+dx*(TopLeft.y-S.y);

        }
        
      pVertices[nCount++].y=TopLeft.y;
      pVertices[nCount].x=P.x;
      pVertices[nCount++].y=P.y;
   }
   *oCount = nCount;
}
void CLIP_Bottom(Vertex *pVertices, unsigned char *oCount, Vertex S, Vertex P/*, unsigned char nVal*/) {
   unsigned char nCount = 0;
   signed long dx,dy;
   dx=P.x-S.x;  dy=P.y-S.y;
   // m = dy/dx -> /m = dx/dy
   // ************OK************
   if ( (S.y<=DownRight.y) && (P.y<=DownRight.y) ) {
      pVertices[nCount].x=P.x;
      pVertices[nCount++].y=P.y;
   }
    
   // *********LEAVING**********
   if ( (S.y<=DownRight.y) && (P.y>DownRight.y) )
   {
      if(dy != 0)
         pVertices[nCount].x=S.x+ gba_div((dx*(DownRight.y-S.y)),dy);
      else {
        pVertices[nCount].x=S.x + (dx*(DownRight.y-S.y));

      }
        
      pVertices[nCount++].y=DownRight.y;
   }
   // ********ENTERING*********
   if ( (S.y>DownRight.y) && (P.y<=DownRight.y) )
   {
      if(dy != 0)
         pVertices[nCount].x=S.x+gba_div((dx*(DownRight.y-S.y)),dy);
      else {
         pVertices[nCount].x=S.x + (dx*(DownRight.y-S.y));

        }
        
      pVertices[nCount++].y=DownRight.y;
      pVertices[nCount].x=P.x;
      pVertices[nCount++].y=P.y;
   }
   *oCount = nCount;
}

void LoadGenSpriteData(unsigned char spriteid, unsigned char nFrame) {
  // load a sprite from the sprite arrays
  // check we are in bounds
  if (spriteid > ELITE_SPRITES_ROOT.nGFXs)
    return;
  // loops over each 8*8 tile making up the sprite...
  // this should be stored in the sprite array too :(
  unsigned short int nSize;
  LZ77GetUnpackedSize(ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->pGFX, &nSize);
  unsigned char * pGFX = (unsigned char*)malloc(nSize*sizeof(unsigned char));

  LZ77UnCompWRAM(ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->pGFX, pGFX);

//	unsigned char * pGFX;
//	pGFX = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->pGFX;
  unsigned short int i;
  // get the size of the object and from this work out how many tiles need to be read in
  //k = <start of frame in map file> = Frame*number of tiles per sprite
  // tile count = (sprites*width*height / 64)
  unsigned short int nTileCount = (ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount *
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nWidth *
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nHeight) >> 6;


  unsigned short int k = nFrame*nTileCount*2;  // 2 because the map is 16 bit and we need to skip every other value

  unsigned short int id;
  unsigned short int index = k;

  // need to load it in blocks of 64...
// unsigned short loop_1, loop_2 = 0;
 // gfx2gba -t8 -M -F radar1.pcx radar2.pcx radar3.pcx radar4.pcx
 // gfx2gba -t8 -c16 -m -F -zt radar1.pcx radar2.pcx radar3.pcx radar4.pcx
// gfx2gba -t8 -c16 -m -F -zt radar.bmp big_s.bmp big_e.bmp big_c.bmp
// gfx2gba -t8 -c16 -m -F -zt -pmaster2.pal controls.bmp
 
  for (i = 0; i < nTileCount; i++) {
    // id = <the tile id in the map file at this position>
    id = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->pGFXMap[index];
//    id = nTileCount; //ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->pGFXMap[index];
    index += 2;
//    loop_2 = 0;
//    for ( loop_1 = 0; loop_1 < 32; loop_1++) {
//    	OAMData[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nOAM+i*32+loop_1] = (pGFX[id*64+loop_2])|(pGFX[id*64+loop_2+1]<<8);
//    	loop_2 +=2;
//    }
    REG_DMA3SAD = (unsigned long int)&(pGFX[id*32]); // 8 bit reads into gfx array
    REG_DMA3DAD = (unsigned long int)&OAMData[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nOAM+i*16]; // 16 bit reads into VRAM
    REG_DMA3CNT = 16 |DMA_16NOW; // 16 bit transfers from gfx array via DMA
  }
  free(pGFX);
}

void LoadGenSprite(unsigned char spriteid, unsigned char copyOAM) {
  // create the sprites .. start at the offset given by the OAM value/16...
  unsigned short int i;
  unsigned short int offset = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nOAM >> 4;
  signed short int temp_x = 0;
  signed short int temp_y = 0;

  unsigned short int max_temp_x = 0;
  unsigned short int increment = 0;
  if (ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount > 1) {
    max_temp_x = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nWidth*
    ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount>>1;
    increment = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nWidth;
  }

//  if (ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount == 1) {
//    temp_x = 0;
//    temp_y = 0;
//  }
  // how many 8 by 8 tiles make up each sprite?
  unsigned short int tilecount88 = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nWidth *
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nHeight >> 6;

  for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i].attribute0 = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nAttribute0;
    sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i].attribute1 = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nAttribute1;

    sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i].attribute2 = PRIORITY(1)|(offset+i*tilecount88);
    MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nX + temp_x,
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nY+ temp_y);

    if (temp_x > max_temp_x) {
      temp_x = 0;
      temp_y +=ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nHeight;
    }
    else
      temp_x += increment;
  }
  if (copyOAM)
    CopyOAM(ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID, ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount);
}



void CopyOAM(unsigned char from, unsigned char to)
{
// some error checking?
//  signed short int* temp;
//  temp = (unsigned short int*)sprites;
//
  unsigned short int loop;
  unsigned short int* temp;
  temp = (unsigned short int*)sprites;
  for(loop = 0; loop < 128*4; loop++)
  {
  	OAM[loop] = temp[loop];
  }
//  from = 0;
//  to = 128;
//  REG_DMA3SAD = (unsigned long int)&temp[from*4];
//  REG_DMA3DAD = (unsigned long int)&OAM[from*4];
//  REG_DMA3CNT = (to-from+1)*4 |DMA_16NOW;

}

void MoveSprite(OAMEntry* sp, signed short int x, signed short int y)
{
    if(x < 0)			//if it is off the left corect
		x = 512 + x;
	if(y < 0)			//if off the top corect
		y = 256 + y;

    /* because x is 9 bits, we need to clear out bits 10-15,
	 so we dont mess up any rotation data */
    x &= 0x01FF;
	sp->attribute1 = sp->attribute1 & 0xFE00;  //clear the old x value
	sp->attribute1 = sp->attribute1 | x;

	sp->attribute0 = sp->attribute0 & 0xFF00;  //clear the old y value
	sp->attribute0 = sp->attribute0 | y;

}

void gfx_draw_menu(unsigned char frame) {
  // draw the scanner... duh
	LoadGenSpriteData(MENU_GFX, frame);
	LoadGenSprite(MENU_GFX, 1);
	SetSpritePriority(MENU_GFX,1);
	CopyOAM(0,127);
}

void gfx_remove_menu(void) {
  unsigned char i;
	unsigned char spriteid = MENU_GFX;
	for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
      240,
      160);
  }
  CopyOAM(0,127);
}


void gfx_draw_scanner(void) {
	// draw the scanner... duh

    LoadOBJPalette(0);
    LoadGenSpriteData(RADAR_GFX, 0);
    LoadGenSprite(RADAR_GFX, 1);
    SetSpritePriority(RADAR_GFX,1);
    CopyOAM(0,127);
    game_draw_flag |= RADAR_DRAWN;
	
	  
}

void gfx_remove_scanner(void) {
    unsigned char i;
    unsigned char spriteid = RADAR_GFX;
    for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
      MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
        240,
        160);
    }
    CopyOAM(0,127);
  if (game_draw_flag & RADAR_DRAWN) {
    game_draw_flag &= ~RADAR_DRAWN;
  }
}

void gfx_remove_big_s(void) {
	unsigned char i;
	unsigned char spriteid = BIG_S_GFX;
	for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
      240,
      160);
  }
  CopyOAM(0,127);
  game_draw_flag &= ~BIG_S_DRAWN;
}

void gfx_remove_big_e(void) {
	unsigned char i;
	unsigned char spriteid = BIG_E_GFX;
	for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
      240,
      160);
  }
  CopyOAM(0,127);
  game_draw_flag &= ~BIG_E_DRAWN;
}

void gfx_remove_big_c(void) {
	unsigned char i;
	unsigned char spriteid = BIG_C_GFX;
	for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    MoveSprite(&sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i],
      240,
      160);
  }
  CopyOAM(0,127);
  game_draw_flag &= ~BIG_C_DRAWN;
}

void remove_all_scanner_gfx(void) {
  gfx_remove_scanner();
  gfx_remove_big_e ();
  gfx_remove_big_s ();
  gfx_remove_big_c ();
}



void gfx_draw_big_s(void) {
	// draw the scanner... duh
	if (!(game_draw_flag & BIG_S_DRAWN)) {
  	LoadGenSpriteData(BIG_S_GFX, 0);
  	LoadGenSprite(BIG_S_GFX, 1);
  	SetSpritePriority(BIG_S_GFX,0);
  	CopyOAM(0,127);
  	game_draw_flag |= BIG_S_DRAWN;
  }
  
}

void gfx_draw_big_e(void) {
	// draw the scanner... duh
	LoadGenSpriteData(BIG_E_GFX, 0);
	LoadGenSprite(BIG_E_GFX, 1);
	SetSpritePriority(BIG_E_GFX,0);
	CopyOAM(0,127);
}

void gfx_draw_big_c(void) {
	// draw the scanner... duh
	LoadGenSpriteData(BIG_C_GFX, 0);
	LoadGenSprite(BIG_C_GFX, 1);
	SetSpritePriority(BIG_C_GFX,0);
	CopyOAM(0,127);
}

void display_dial_bar (unsigned char len, unsigned char x, unsigned char y) {
	
	unsigned char i,j;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < len; j++) 
			gba_drawpixel(x+j,y+i);
	}
	
}


void LoadOBJPalette(unsigned char pal) {

//	REG_DMA3SAD = (unsigned long)&_binary_img_master_pal_start[0];
//  REG_DMA3DAD = (unsigned long)&OBJPaletteMem[0];
//  REG_DMA3CNT = 9 | DMA_16NOW;

////	REG_DMA3SAD = 0;
////	REG_DMA3DAD = 0;
////	REG_DMA3CNT = 0;
	unsigned short int i;
	unsigned short int j;
	j = 0;
	for (i = 0; i< 16; i++) {
	  if (pal == 0)
		  OBJPaletteMem[i] = _binary_master_pal_start[j]|(_binary_master_pal_start[j+1]<<8);
		else
		  OBJPaletteMem[i] = _binary_master2_pal_start[j]|(_binary_master2_pal_start[j+1]<<8);
		j+=2;
	}  
//  OBJPaletteMem[0] = 0x421C; // set the invisible colour to peach
//  OBJPaletteMem[TEXT_OL_COL] = 0x0000; // set the text outline to black
}

void draw_square (unsigned char x, unsigned char y) {
	// draw a 4*4 square
	unsigned short int i;
	for (i = 0; i< 4; i++) {
		gba_scanline(x,160-(y+i),x+2);
	}
}

void SetSpritePriority(unsigned char spriteid, unsigned char priority) {
  unsigned char i;
  if (priority > 3)
    priority = 3;
  unsigned short int offset = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nOAM >> 4;
  unsigned char tilecount88 = ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nWidth *
      ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nHeight >> 6;
  for (i = 0; i < ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteCount; i++) {
    sprites[ELITE_SPRITES_ROOT.pSpriteTable[spriteid].pSpriteMap->nSpriteID+i].attribute2 = PRIORITY(priority)|(offset+i*tilecount88);
  }
}


void gfx_draw_spot (unsigned char x, unsigned char y) {
	// draw a spot like the planet, a + shape - used for the scanner
	gba_drawpixel(x, y);
	gba_drawpixel(x+1, y);
	gba_drawpixel(x-1, y);
	gba_drawpixel(x, y+1);
	gba_drawpixel(x, y-1);
	
}


void gfx_draw_dot (unsigned char x, unsigned char y) {
	// draw a dot consisting of 2x2 pixels

	gba_drawpixel(x, y);
	gba_drawpixel(x+1, y);
	gba_drawpixel(x, y+1);
	gba_drawpixel(x+1, y+1);
	
}


void gfx_clear_status_area(void) {
  gba_setcolour(0x00);
  gba_triangle(0, DownRight.y, 239, DownRight.y,
               0, 159);
  
  gba_triangle(           239, DownRight.y,
               0, 159,    239, 159);
  
}

void gfx_draw_selection_square(unsigned char x, unsigned char y) {
  // draw a square at the x,y position that is 17 by 17 to highlight a selected menu item
  
  gba_horizline(x,y+DownRight.y,18);
  gba_horizline(x,y+DownRight.y+17,18);
  
  gba_vertline(x,y+DownRight.y,17);
  gba_vertline(x+17,y+DownRight.y,17);
  
//  gba_drawline(x,y+DownRight.y,x+17,y+DownRight.y);
//  gba_drawline(x,y+17+DownRight.y,x+17,y+17+DownRight.y);
//  gba_drawline(x,160-(y+DownRight.y),x,160-(y+17+DownRight.y));
//  gba_drawline(x+18,160-(y+DownRight.y),x+18,160-(y+17+DownRight.y));
  
}

void print_multi_text(unsigned char x, unsigned char y, const char *txt) {
	char strbuf[100];
	const char *str;
	char *bptr;
	signed short int len;
	signed short int pos;
	unsigned char maxlen = 28;
	
	
	str = txt;
	len = strlen(txt);
	
	while (len > 0)
	{
		pos = maxlen;
		if (pos > len)
			pos = len;

		while ((str[pos] != ' ') && (str[pos] != ',') &&
			   (str[pos] != '.') && (str[pos] != '\0'))
		{
			pos--;
		}

		len = len - pos - 1;
	
		for (bptr = strbuf; pos >= 0; pos--)
			*bptr++ = *str++;

		*bptr = '\0';
    
    gba_print(x,y,strbuf);
		y ++;
	}
}




void gfx_centre_text(unsigned char y, char *txt) {
  signed short int len;  
  signed short int x;  
  char str[31];

  len = strlen(txt);
  
  if (len > 30) {
    print_multi_text(0,y,txt);    
  } else {
    // strip off trailing blanks
    strcpy(str, txt);
    while (str[len-1] == ' ') {
        str[len-1] = 0;
        len--;
    }
    x = (30 - len)>>1;
    gba_print(x,y,str);
  }
    
}

void gfx_centre_text_masked(unsigned char y, const char *txt) {
  signed short int len;  
  signed short int x;  
  char str[31];

  len = strlen(txt);
  
  if (len > 30) {
    print_multi_text(0,y,txt);    
  } else {
    // strip off trailing blanks
    strcpy(str, txt);
    while (str[len-1] == ' ') {
        str[len-1] = 0;
        len--;
    }
    x = (30 - len)>>1;
    gba_print_masked(x,y,(char*)str);
  }
    
}

void gfx_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {
  // draw a line, but check the stuff needed first
  if (y1 != y2)
    gba_drawline(x1,(160 - y1),x2,(160 - y2));
  else
    gba_drawline(x1,y1,x2,y2);    
}


void gfx_line_clip(signed long int x1, signed long int y1, signed long int z1,
                   signed long int x2, signed long int y2, signed long int z2) {
  
  // clip a line to the screen. This is a hacked version of the 
  // triangle clipping code, it just does each step once and updates
  // the 2 points directly each time. If after a clip there are no points left
  // on screen, then the routine returns immediately.
  
  VertexFixed S,P,S1,P1;
  signed long dx,dy;
  unsigned char nCount = 0;
  
  S.x = x1; S.y = y1; S.z = z1;
  P.x = x2; P.y = y2; P.z = z2;
  
  // clip to the front of the screen (Z clip)
  P1 = P;
  S1 = S;
  // OK - leave as is
  if ( (S.z >= ZMin) && (P.z >= ZMin) ) {
    nCount++;
  }
  
  // If it's leaving the viewport, i.e. S inside P outside, 
  // Store S as is and recalculate P
  if ( (S.z>=ZMin) && (P.z < ZMin) )
  {
    if ((P.z-S.z)!= 0) {
      P1.x = S.x + gba_div(((P.x-S.x)*(ZMin-S.z)),(P.z-S.z));
      P1.y = S.y + gba_div(((P.y-S.y)*(ZMin-S.z)),(P.z-S.z));
    } else {
      P1.x = S.x + ((P.x-S.x)*(ZMin-S.z));
      P1.y = S.y + ((P.y-S.y)*(ZMin-S.z));
    }
    P1.z = ZMin;
    nCount++;
  }
  
  // If it's entering the viewport. i.e. S is outside but P inside
  // recalculate S and keep P as is
  if ( (S.z<ZMin) && (P.z>=ZMin) )
  {
    if ((P.z-S.z) != 0) {
      S1.x = S.x + gba_div(((P.x-S.x)*(ZMin-S.z)),(P.z-S.z));
      S1.y = S.y + gba_div(((P.y-S.y)*(ZMin-S.z)),(P.z-S.z));
    } else {
      S1.x = S.x + ((P.x-S.x)*(ZMin-S.z));
      S1.y = S.y + ((P.y-S.y)*(ZMin-S.z));
    }
    
    S1.z = ZMin;
    nCount++;
  }
  // check if we have anything left on screen...
  if (nCount == 0)
   return;
  nCount = 0;
   
  S = S1;
  P = P1;
    
  // clip to the left of the screen...
  
  P1 = P;
  S1 = S;
  
  // OK - do nothing
  if ( (S.x>=TopLeft.x) && (P.x>=TopLeft.x) ) {
    nCount++;
  }
  // LEAVING - leave S as is and set P to be the edge of the screen 
  if ( (S.x>=TopLeft.x) && (P.x<TopLeft.x) ) {
    P1.x=TopLeft.x;
    if ((P.x-S.x) != 0)
      P1.y=S.y+ gba_div(((P.y-S.y)*(TopLeft.x-S.x)),(P.x-S.x));
    else  
      P1.y=S.y+ ((P.y-S.y)*(TopLeft.x-S.x));
    
   nCount++;
  }
  
  // ENTERING - set S to the edge of the screen and leave P as is
  if ( (S.x<TopLeft.x) && (P.x>=TopLeft.x) )  {
    S1.x=TopLeft.x;
    if ((P.x-S.x) != 0)
      S1.y=S.y+ gba_div(((P.y-S.y)*(TopLeft.x-S.x)),(P.x-S.x));
    else
      S1.y=S.y+ ((P.y-S.y)*(TopLeft.x-S.x));
    
   nCount++;
  } 
  // check if we have anything left on screen... 
  if (nCount == 0)
   return;
  nCount = 0;
  
  P = P1;
  S = S1;
   
  // clip to the right of the screen...
  dx=P.x-S.x;  dy=P.y-S.y;
  P1 = P;
  S1 = S;
  // OK - do nothing
  if ( (S.x<=DownRight.x) && (P.x<=DownRight.x) ) {
      nCount++;
   }
  // LEAVING - leave S and set P to the right edge of screen
   if ( (S.x<=DownRight.x) && (P.x>DownRight.x) )
   {
      P1.x=DownRight.x;
      if (dx!= 0)
        P1.y=S.y+gba_div((dy*(DownRight.x-S.x)),dx);
      else
        P1.y=S.y+(dy*(DownRight.x-S.x));
        
      nCount++;
   }
   // ENTERING - set S to the edge of screen, leaave P as is
   if ( (S.x>DownRight.x) && (P.x<=DownRight.x) )
   {
      S1.x=DownRight.x;
      if (dx != 0)
        S1.y=S.y+gba_div((dy*(DownRight.x-S.x)),dx);
      else
        S1.y=S.y+(dy*(DownRight.x-S.x));
      nCount++;
   }
   // check if we have anything left on screen... 
  if (nCount == 0)
   return;
  nCount = 0;
  P = P1;
  S = S1;
   
  // clip to the top of screen
  dx=P.x-S.x;  dy=P.y-S.y;
  P1 = P;
  S1 = S;
   // m = dy/dx -> /m = dx/dy
   // OK do nothing
    if ( (S.y>=TopLeft.y) && (P.y>=TopLeft.y) ) {
      nCount++;
    }
   // LEAVING - keep S and reset P
   if ( (S.y>=TopLeft.y) && (P.y<TopLeft.y) )
   {
      if(dy)
         P1.x=S.x+gba_div(dx*(TopLeft.y-S.y),dy);
      else {
         P1.x=S.x + dx*(TopLeft.y-S.y);
        }
        
      P1.y=TopLeft.y;
      nCount++;
   }
   // ENTERING - reset S and keep P
   if ( (S.y<TopLeft.y) && (P.y>=TopLeft.y) )
   {
      if(dy)
         S1.x=S.x+gba_div(dx*(TopLeft.y-S.y),dy);
      else {
         S1.x=S.x+dx*(TopLeft.y-S.y);

        }
        
      S1.y=TopLeft.y;
      nCount++;
   }
   // check if we have anything left on screen... 
  if (nCount == 0)
   return;
  nCount = 0;
  P = P1;
  S = S1;
  
  // clip to the bottom of the screen
  dx=P.x-S.x;  dy=P.y-S.y;
  P1 = P;
  S1 = S;
   // m = dy/dx -> /m = dx/dy
   // OK do nothing
   if ( (S.y<=DownRight.y) && (P.y<=DownRight.y) ) {
      nCount++;
   }
   // LEAVING - keep S and reset P
   if ( (S.y<=DownRight.y) && (P.y>DownRight.y) )
   {
      if(dy != 0)
         P1.x=S.x+ gba_div((dx*(DownRight.y-S.y)),dy);
      else {
        P1.x=S.x + (dx*(DownRight.y-S.y));

      }
        
      P1.y=DownRight.y;
      nCount++;
   }
   // ENTERING - keep P and reset S
   if ( (S.y>DownRight.y) && (P.y<=DownRight.y) )
   {
      if(dy != 0)
         S1.x=S.x+gba_div((dx*(DownRight.y-S.y)),dy);
      else {
         S1.x=S.x + (dx*(DownRight.y-S.y));

        }
      S1.y=DownRight.y;
      nCount++;
   }  
    // check if we have anything left on screen... 
    if (nCount == 0)
      return;
    nCount = 0;
   P = P1;
   S = S1;
  
//  gfx_line(S.x, S.y, P.x, P.y);
  gfx_slow_line(S.x, S.y, P.x, P.y);
}
/////////////////////Line//////////////////////////
//An implementation of the Bresenham Line algorithm
void gfx_slow_line(int x1, int y1, int x2, int y2)
{
	//Variables
	int i, deltax, deltay, numpixels;
	int d, dinc1, dinc2;
	int xa, xb, xinc1, xinc2;
	int ya, yb, yinc1, yinc2;

	//Calculate deltaX and deltaY
	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);

	//Init vars
	if(deltax >= deltay)
	{
		//If x is independent variable
		numpixels = deltax + 1;
		d = (2 * deltay) - deltax;
		dinc1 = deltay << 1;
		dinc2 = (deltay - deltax) << 1;
		xinc1 = 1;
		xinc2 = 1;
		yinc1 = 0;
		yinc2 = 1;
	}
	else
	{
		//If y is independant variable
		numpixels = deltay + 1;
		d = (2 * deltax) - deltay;
		dinc1 = deltax << 1;
		dinc2 = (deltax - deltay) << 1;
		xinc1 = 0;
		xinc2 = 1;
		yinc1 = 1;
		yinc2 = 1;
	}

	//Move the right direction
	if(x1 > x2)
	{
		xinc1 = -xinc1;
		xinc2 = -xinc2;
	}
	if(y1 > y2)
	{
		yinc1 = -yinc1;
		yinc2 = -yinc2;
	}

    // lines are symetrical about their mid-points
    // Store two x and y values, (xa, ya) and (xb, yb). 
    // Have each pair start on either end of the line. 
	xa = x1;
	ya = y1;
    
    xb = x2;
    yb = y2;
    
    
    // Draw the pixels, half the expected amount due to symmetry
    // add 3 due to loss of precision dividing by 2
	numpixels = (numpixels+3)>>1;
	for(i = 1; i < numpixels; i++)
	{
	    // For each pass through the loop draw the pixel at both points
		gba_drawpixel(xa,ya);
		gba_drawpixel(xb,yb);

        // add to xa,ya end and subtract from xb,yb end 
		if(d < 0)
		{
		    d = d + dinc1;
            xa = xa + xinc1;
			xb = xb - xinc1;
			ya = ya + yinc1;
			yb = yb - yinc1;
		}
		else
		{
		    d = d + dinc2;
            xa = xa + xinc2;
			xb = xb - xinc2;
			ya = ya + yinc2;
			yb = yb - yinc2;
		}
	}
}

void load_ship_palette(void) {
  u8 val;
  if (game_state_flag & GBA_PALETTE)
    val = 1;
  else
    val = 0;
  u16 i;
  for(i = 1; i < 256; i++) {
  gba_setpalette( i,
          ELITE_SHIPS.pPalette[val][i].R,
          ELITE_SHIPS.pPalette[val][i].G,
          ELITE_SHIPS.pPalette[val][i].B);
  }
  gba_setpalette( 0, 0,0,0);
  
  gba_setpalette( GFX_COL_DARK_RED   , 31, 0, 0);
  gba_setpalette( GFX_COL_LIGHT_BLUE , 19, 29, 28);
  gba_setpalette( GFX_COL_GREY       , 25, 25, 25);
  gba_setpalette( GFX_COL_DARK_GREY  , 15, 15, 15);
  gba_setpalette( GFX_COL_WHITE      , 31, 31, 31);
  gba_setpalette( GFX_COL_LIGHT_GREEN, 10, 31, 11);
  gba_setpalette( GFX_COL_SHIELD_PINK, 27,  9, 27);
  gba_setpalette( GFX_COL_PIN_PINK,    31,  15, 15);
  gba_setpalette( GFX_COL_ORANGE     , 29, 24,  5); // 29, 26, 5); // TM
  gba_setpalette( GFX_COL_DARK_GREEN , 9, 20, 7);
  gba_setpalette( GFX_COL_BROWN      , 22, 12, 6);
  gba_setpalette( GFX_COL_GOLD       , 30, 29, 10);
  gba_setpalette( GFX_COL_MED_BLUE   ,  4, 23, 27);
  gba_setpalette( GFX_COL_DARK_BLUE  , 2, 10, 27);
  gba_setpalette( GFX_COL_DARK_BLUE2 , 7, 13, 22);
  gba_setpalette( GFX_COL_SUN_YELLOW , 31,30,21);
  gba_setpalette( GFX_COL_LASER_RED  , 31, 0, 0);
  gba_setpalette( GFX_COL_PLANET_GREEN        , 11, 25, 8);
  gba_setpalette( GFX_COL_PLANET_MED_BLUE     ,  4, 23, 27);
  gba_setpalette( GFX_COL_PLANET_DARK_BLUE    ,  2, 10, 27);
  gba_setpalette( GFX_COL_PLANET_PALE_BLUE    ,  7, 13, 22);
  gba_setpalette( GFX_COL_PLANET_ORANGE       , 27, 12, 2);
  gba_setpalette( GFX_COL_PLANET_PALE_VIOLET  , 24, 13, 16);
  gba_setpalette( GFX_COL_PLANET_BROWN        , 24, 16, 10);
  gba_setpalette( GFX_COL_PLANET_PORNY_PURPLE , 31, 10, 28);
  
}



