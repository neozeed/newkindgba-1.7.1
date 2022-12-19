#include "stars.h"
#include "vector.h"
#include "elite.h"
#include "gfx.h"
#include "gbalib_asm.h"
#include "random.h"


#define WITCH_SPACE_STARS 4
#define WITCH_SPACE_FIX_STARS 0
#define NORMAL_STARS 24
#define NORMAL_FIX_STARS 14 

void front_starfield(int fix_stars);
void rear_starfield (int fix_stars);
void side_starfield (int fix_stars);

VertexFixed stars[NORMAL_STARS]; 

void create_new_stars (void)
{
	unsigned char i;
	unsigned char nstars;
	
	nstars = (game_state_flag & WITCHSPACE)? WITCH_SPACE_STARS : NORMAL_STARS;

	for (i = 0; i < nstars; i++)
	{
		stars[i].x = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 8)<<8;
		stars[i].y = ((GetRand(DownRight.y) - (DownRight.y>>1)) | 4)<<8;
		stars[i].z = (GetRand(255) | 0x90)<<8;
	}
    if (game_draw_flag & WARP_STARS)
      game_draw_flag &= ~WARP_STARS;
    
}

void create_new_fix_star (unsigned char i, signed long int sx, signed long int sy) {
	if ((sx > DownRight.x) || (sx < 0) ||
	(sy > DownRight.y) || (sy < 0))
		{
			stars[i].x = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 8)<<8;
			stars[i].y = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 4)<<8;
			stars[i].z = (GetRand(255) | 0x90)<<8;
		}

}

//void create_new_stars_far (void)
//{
//	unsigned char i;
//	unsigned char nstars;
//	
//	nstars = (game_state_flag & WITCHSPACE)? WITCH_SPACE_STARS : NORMAL_STARS;
//
//	for (i = 0; i < nstars; i++)
//	{
//		stars[i].x = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 8)<<4;
//		stars[i].y = ((GetRand(DownRight.y) - (DownRight.y>>1)) | 4)<<4;
//		stars[i].z = (GetRand(255) | 0x90)<<8;
//	}
//    if (game_draw_flag & WARP_STARS)
//      game_draw_flag &= ~WARP_STARS;
//    
//}


void rear_starfield (int fix_stars)
{
	signed long int xx,yy,zz;
	signed long int sx;
	signed long int sy;
	unsigned char i, ifirst, ilast;
	unsigned char nstars, nfixstars;
	
	if (game_state_flag & WITCHSPACE) {
        nstars = WITCH_SPACE_STARS;
        nfixstars = WITCH_SPACE_FIX_STARS;
    }
    else {
        nstars = NORMAL_STARS;
        nfixstars = NORMAL_FIX_STARS;
    }


	signed short delta = (game_draw_flag & WARP_STARS)?50<<7:flight_speed<<7;	
	signed short alpha = -flight_roll;
	signed short beta = -flight_climb;
	
	ifirst = fix_stars? 0: nfixstars+1;
	ilast = fix_stars? nfixstars: nstars-1;
	
    gba_setcolour(GFX_COL_WHITE);
	for (i = ifirst; i <= ilast; i++)
	{
//    	gba_setcolour((i < nfixstars)? GFX_COL_GREY: GFX_COL_WHITE);
		/* Plot the stars in their current locations... */
		sy = stars[i].y>>8;
		sx = stars[i].x>>8;
		zz = stars[i].z>>8;

		sx += (DownRight.x>>1);
		sy += (DownRight.y>>1);
		
		if ( (!(game_draw_flag & WARP_STARS) || (i < nfixstars))&&
			(sx >= TopLeft.x) && (sx <= DownRight.x) &&
			(sy >= TopLeft.y) && (sy <= DownRight.y))
		{
			
			gba_drawpixel(sx, sy);

			if (zz < 0xC0)
				gba_drawpixel(sx+1, sy);

			if (zz < 0x90)
			{
				gba_drawpixel (sx, sy+1);
				gba_drawpixel (sx+1, sy+1);
			}
			
		}


		/* Move the stars to their new locations...*/
		
        if (i >= nfixstars) {
    		stars[i].z += delta>>1;
    		yy = stars[i].y - gba_div(stars[i].y * delta, stars[i].z);
    		xx = stars[i].x - gba_div(stars[i].x * delta, stars[i].z);
        }
        else {
            xx = stars[i].x;
            yy = stars[i].y;
        }
    	
        zz = stars[i].z>>8;
		
		yy = yy + ((xx * alpha)>>8);
		xx = xx - ((yy * alpha)>>8);
		yy = yy + (beta<<8);
		
		if ((game_draw_flag & WARP_STARS) && (i >= nfixstars))
		{
		    signed long int ex,ey;
			ey = yy>>8;
			ex = xx>>8;
			ex += (DownRight.x>>1);
		    ey += (DownRight.y>>1);
		    
			if ((sx > TopLeft.x) && (sx < DownRight.x) &&
                (sy > TopLeft.y) && (sy < DownRight.y) &&
                (ex > TopLeft.x) && (ex < DownRight.x) &&
                (ey > TopLeft.y) && (ey < DownRight.y)) 
			{
			  gfx_slow_line(sx,sy,ex,ey); 
//			  if (sy != ey)
//			    gba_drawline(sx,(160 - sy),ex,(160 - ey));
//			  else
//			    gba_drawline(sx,sy,ex,ey); 
			}
		}
		
		stars[i].y = yy;
		stars[i].x = xx;
		
		sy = yy>>8;
		sy+=(DownRight.y>>1);
		
		if (i < nfixstars)
		    create_new_fix_star(i, sx, sy);
		else
            if ((zz >= 200) || (sy >= DownRight.y) || (sy <= TopLeft.y))
    		{
    			stars[i].z = ((rand255() & 127) + 51)<<8;
    			
                if (rand255() & 1)
    			{
    				// start at the top or bottom and random the x
    				stars[i].x = (GetRand(DownRight.x) - (DownRight.x>>1))<<8;
    //				stars[i].y = ((rand255() & 1) ? -(DownRight.y): (DownRight.y))<<8;
    				stars[i].y = ( (rand255() & 1) ? -(DownRight.y>>1) : (DownRight.y>>1))<<8;
    			}
    			else
    			{
    				// start at the left or right and random the y
    				stars[i].x = ((rand255() & 1) ? -(DownRight.x>>1) : (DownRight.x>>1))<<8;
    				stars[i].y = (GetRand(DownRight.y) - DownRight.y)<<8;
    			}
    		}

	}
}

void front_starfield (int fix_stars)
{
	
	signed long int xx,yy,zz;
	signed long int sx;
	signed long int sy;
	unsigned char i, ifirst, ilast;
	unsigned char nstars, nfixstars;
	
	if (game_state_flag & WITCHSPACE) {
        nstars = WITCH_SPACE_STARS;
        nfixstars = WITCH_SPACE_FIX_STARS;
    }
    else {
        nstars = NORMAL_STARS;
        nfixstars = NORMAL_FIX_STARS;
    }

	signed short delta = (game_draw_flag & WARP_STARS)?50<<7:flight_speed<<7;
	signed short alpha = flight_roll;
	signed short beta = flight_climb;
	
	ifirst = fix_stars? 0: nfixstars+1;
	ilast = fix_stars? nfixstars: nstars-1;
	
    gba_setcolour(GFX_COL_WHITE);
	for (i = ifirst; i <= ilast; i++)
	{
//	    gba_setcolour((i < nfixstars)? GFX_COL_GREY: GFX_COL_WHITE);
		/* Plot the stars in their current locations... */
		sy = stars[i].y>>8;
		sx = stars[i].x>>8;
		zz = stars[i].z>>8;

		sx += (DownRight.x>>1);
		sy += (DownRight.y>>1);
	
		if ((!(game_draw_flag & WARP_STARS) || (i < nfixstars)) &&
			(sx >= TopLeft.x) && (sx <= DownRight.x) &&
			(sy >= TopLeft.y) && (sy <= DownRight.y))
		{
			
			gba_drawpixel(sx, sy);

			if (zz < 0xC0)
				gba_drawpixel(sx+1, sy);

			if (zz < 0x90)
			{
				gba_drawpixel (sx, sy+1);
				gba_drawpixel (sx+1, sy+1);
			}
		}


		/* Move the stars to their new locations...*/


        if (i >= nfixstars) {
    		stars[i].z -= delta>>1;
    		if (stars[i].z != 0)
    			yy = stars[i].y + (gba_div(stars[i].y * delta, stars[i].z));
    		else
    			yy = stars[i].y + (stars[i].y * delta);
                
            if (stars[i].z != 0)
    			xx = stars[i].x + (gba_div(stars[i].x * delta, stars[i].z));
    		else
    			xx = stars[i].x + (stars[i].x * delta);
        }
        else {
            xx = stars[i].x;		
            yy = stars[i].y;
        }		
    	
        zz = stars[i].z>>8;
		yy = yy + ((xx * alpha)>>8);
		xx = xx - ((yy * alpha)>>8);

		yy = yy + (beta<<8);

		stars[i].y = yy;
		stars[i].x = xx;
        
        if ((game_draw_flag & WARP_STARS) && (i >= nfixstars)) {
            signed long int ex,ey;
			ex = xx>>8;
			ey = yy>>8;
			ex += (DownRight.x>>1);
		    ey += (DownRight.y>>1);
		    
			if ((sx > TopLeft.x) && (sx < DownRight.x) &&
                (sy > TopLeft.y) && (sy < DownRight.y) &&
                (ex > TopLeft.x) && (ex < DownRight.x) &&
                (ey > TopLeft.y) && (ey < DownRight.y)) 
			{
			  gfx_slow_line(sx,sy,ex,ey); 
//			  if (sy != ey)
//			    gba_drawline(sx,(160 - sy),ex,(160 - ey));
//			  else
//			    gba_drawline(sx,sy,ex,ey); 
			}
        }
        
		sx = (xx>>8)+(DownRight.x>>1);
		sy = (yy>>8)+(DownRight.y>>1);

		if (i < nfixstars)
	        create_new_fix_star(i, sx, sy);
	    else
            if ((sx > DownRight.x) || (sx < 0) ||
    			(sy > DownRight.y) || (sy < 0) || (zz < 16) || (zz > 345))
    		{
    			stars[i].x = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 8)<<8;
    			stars[i].y = ((GetRand(DownRight.x) - (DownRight.x>>1)) | 4)<<8;
    			stars[i].z = (GetRand(255) | 0x90)<<8;
    		}

	}
}

void side_starfield (int fix_stars)
{
	unsigned char i, ifirst, ilast;
	unsigned char nstars, nfixstars;
	signed long int xx,yy,zz;
	signed long int sx;
	signed long int sy;
	
    signed long int delt8;
	
	signed short delta = (game_draw_flag & WARP_STARS)?50<<7:flight_speed<<7;
	signed short alpha = flight_roll;
	signed short beta = flight_climb;
	
	if (game_state_flag & WITCHSPACE) {
        nstars = WITCH_SPACE_STARS;
        nfixstars = WITCH_SPACE_FIX_STARS;
    }
    else {
        nstars = NORMAL_STARS;
        nfixstars = NORMAL_FIX_STARS;
    }
	
	if (current_screen == SCR_LEFT_VIEW)
	{
		delta = -delta;
		alpha = -alpha;
		beta = -beta;
	} 
	
	ifirst = fix_stars? 0: nfixstars+1;
	ilast = fix_stars? nfixstars: nstars-1;
	
    gba_setcolour(GFX_COL_WHITE);
	for (i = ifirst; i <= ilast; i++)
	{
//     	gba_setcolour((i < nfixstars)? GFX_COL_GREY: GFX_COL_WHITE);
		
		sy = stars[i].y>>8;
		sx = stars[i].x>>8;
		zz = stars[i].z>>8;

		sx += (DownRight.x>>1);
		sy += (DownRight.y>>1);
		
		if ( (!(game_draw_flag & WARP_STARS) || (i < nfixstars)) &&
			(sx >= TopLeft.x) && (sx <= DownRight.x) &&
			(sy >= TopLeft.y) && (sy <= DownRight.y))
		{
			gba_drawpixel(sx, sy);

			if (zz < 0xC0)
				gba_drawpixel(sx+1, sy);

			if (zz < 0x90)
			{
				gba_drawpixel (sx, sy+1);
				gba_drawpixel (sx+1, sy+1);
			}
		}
    
    
		yy = stars[i].y;
		xx = stars[i].x;
		zz = stars[i].z>>8;
		
        if (i >= nfixstars) {
            if (zz != 0)
    		  delt8 = gba_div((delta * 32)<<2, zz);
    		else
    		  delt8 = (delta * 32)>>1;
    		
    		xx = xx + delt8;
        }

		xx += (yy * beta)>>8;	// (/ 256)
		yy -= (xx * beta)>>8; // (/ 256)

//		xx += ((yy / 256) * (alpha / 256)) * (-xx);
//		yy += ((yy / 256) * (alpha / 256)) * (yy);

        xx += ((yy >> 8) * alpha * -xx)>>24;
        yy += ((yy >> 8) * alpha * yy)>>24;
    
		yy += alpha<<8; 

		stars[i].y = yy;
		stars[i].x = xx;
        
        if ((game_draw_flag & WARP_STARS) && (i >= nfixstars)) {
            signed long int ex,ey;
			ex = xx>>8;
			ey = yy>>8;
			ex += (DownRight.x>>1);
		    ey += (DownRight.y>>1);
		    
			if ((sx > TopLeft.x) && (sx < DownRight.x) &&
                (sy > TopLeft.y) && (sy < DownRight.y) &&
                (ex > TopLeft.x) && (ex < DownRight.x) &&
                (ey > TopLeft.y) && (ey < DownRight.y)) 
			{
			  gfx_slow_line(sx,sy,ex,ey); 
//			  if (sy != ey)
//			    gba_drawline(sx,(160 - sy),ex,(160 - ey));
//			  else
//			    gba_drawline(sx,sy,ex,ey); 
			}
        }
        

		sx = (xx>>8)+(DownRight.x>>1);
		sy = (yy>>8)+(DownRight.y>>1);
		
		if (i < nfixstars)
	        create_new_fix_star(i, sx, sy);
	    else
    		if ((sx > DownRight.x) || (sx < 0))
    //		if (abs(stars[i].x>>8) >= (DownRight.x>>1))
    		{
    			stars[i].y = ( GetRand(DownRight.y) - (DownRight.y>>1) )<<8;
    			stars[i].x = ( (current_screen == SCR_LEFT_VIEW) ? (DownRight.x>>1) : -(DownRight.x>>1) )<<8;
    			stars[i].z = (rand255() | 8)<<8;
    		}
    		else if ((sy > DownRight.y) || (sy < 0))
    		{
    		  stars[i].x = (GetRand(DownRight.x) - (DownRight.x>>1))<<8;
    			stars[i].y = ((alpha > 0) ? -(DownRight.y>>1) : (DownRight.y>>1))<<8;
    			stars[i].z = (rand255() | 8)<<8;
    		} 
	}
}

void update_starfield (int fix_stars)
{
	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
		case SCR_INTRO_ONE:
		case SCR_INTRO_TWO:
		case SCR_ESCAPE_POD:
		case SCR_BREAK_PATTERN:
        case SCR_BREAK_PATTERN_ESC_POD:
			front_starfield(fix_stars);
			break;
		
		case SCR_REAR_VIEW:
		case SCR_GAME_OVER:
			rear_starfield(fix_stars);
			break;
		
		case SCR_LEFT_VIEW:
		case SCR_RIGHT_VIEW:
			side_starfield(fix_stars);
			break;
	}
}


/*
 * When we change view, flip the stars over so they look like other stars.
 */
void flip_stars (void)
{
	unsigned char i;
	unsigned char nstars;
	signed long int sx;
	signed long int sy;
	
	nstars = (game_state_flag & WITCHSPACE)? WITCH_SPACE_STARS : NORMAL_STARS;
	for (i = 0; i < nstars; i++)
	{
		sy = stars[i].y;
		sx = stars[i].x;
		stars[i].x = sy;
		stars[i].y = sx;
	}
}





