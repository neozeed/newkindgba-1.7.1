/*
 * Elite - The New Kind.
 *
 * Reverse engineered from the BBC disk version of Elite.
 * Additional material by C.J.Pinder.
 *
 * The original Elite code is (C) I.Bell & D.Braben 1984.
 * This version re-engineered in C by C.J.Pinder 1999-2001.
 *
 * email: <christian@newkind.co.uk>
 *
 *
 */

/*
 * pilot.c
 *
 * The auto-pilot code.  Used for docking computer and for
 * flying other ships to and from the space station.
 */
 
/*
 * In the original Elite this code was mixed in with the tactics routines.
 * I have split it out to make it more understandable and easier to maintain.
 */
  
// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include "gfx.h"
#include "elite.h"
#include "vector.h"
#include "space.h"
#include "shipdata.h"
#include "main.h"
#include "gbalib_asm.h"
#include "sound.h"

//#include "krawall.h"


/*
 * Fly to a given point in space.
 */

void fly_to_vector (Object *ship, VertexFixed vec)
{
	VertexFixed nvecf;
	Vertex nvec;
	signed long direction;
	signed long dir;
	signed long rat;
	signed long rat2;
	signed long cnt2;

	rat = 3;
	rat2 = 0xAA;
	cnt2 = 0x338;
  
    nvecf.x = vec.x>>8;
    nvecf.y = vec.y>>8;
    nvecf.z = vec.z>>8;
  
	UnitLengthFixed(&nvecf);
	nvec.x = nvecf.x;
  nvec.y = nvecf.y;
  nvec.z = nvecf.z;
  
  // div 256 because Unit Length is Fixed point * 256
	direction = vector_dot_product (&nvec, &ship->rotmat[2]); 
	direction >>= FIXEDPOINT;
	if (direction < -0x2AC)
		rat2 = 0;

	dir = vector_dot_product (&nvec, &ship->rotmat[1]);
    dir >>= FIXEDPOINT;
	if (direction < -0x373)
	{
		ship->rotx = (dir < 0) ? 7 : -7;
		ship->rotz = 0;
		return; 
	}

	ship->rotx = 0;
	
	if ((abs(dir) * 2) >= rat2)
	{
		ship->rotx = (dir < 0) ? rat : -rat;
	}
		
	if (abs(ship->rotz) < 16)
	{
		dir = vector_dot_product (&nvec, &ship->rotmat[0]);
        dir >>= FIXEDPOINT;
		ship->rotz = 0;

		if ((abs(dir) * 2) >= rat2)
		{
			ship->rotz = (dir < 0) ? rat : -rat;

			if (ship->rotx < 0)
				ship->rotz = -ship->rotz;
		}		
	}

	if (direction <= -0xAB)
	{
		ship->acceleration = -1;
		return;
	}

	if (direction >= cnt2)
	{
		ship->acceleration = 3;
		return;
	}
	
}



/*
 * Fly towards the planet.
 */

void fly_to_planet (Object *ship)
{
	VertexFixed vec;

	vec.x = universe[0].location.x - ship->location.x;
	vec.y = universe[0].location.y - ship->location.y;
	vec.z = universe[0].location.z - ship->location.z;

	fly_to_vector (ship, vec);	
}


/*
 * Fly to a point in front of the station docking bay.
 * Done prior to the final stage of docking.
 */


void fly_to_station_front (Object *ship)
{
	VertexFixed vec;

	vec.x = universe[1].location.x - ship->location.x;
	vec.y = universe[1].location.y - ship->location.y;
	vec.z = universe[1].location.z - ship->location.z;

	vec.x += (universe[1].rotmat[2].x * 2500);
	vec.y += (universe[1].rotmat[2].y * 2500);
	vec.z += (universe[1].rotmat[2].z * 2500);

	fly_to_vector (ship, vec);	
}


/*
 * Fly towards the space station.
 */

void fly_to_station (Object *ship)
{
	VertexFixed vec;

	vec.x = universe[1].location.x - ship->location.x;
	vec.y = universe[1].location.y - ship->location.y;
	vec.z = universe[1].location.z - ship->location.z;

	fly_to_vector (ship, vec);	
}


/*
 * Final stage of docking.
 * Fly into the docking bay.
 */
 
void fly_to_docking_bay (Object *ship)
{
	Vertex vec;
	signed long dir;

	vec.x = (ship->location.x - universe[1].location.x)>>8;
	vec.y = (ship->location.y - universe[1].location.y)>>8;
	vec.z = (ship->location.z - universe[1].location.z)>>8;

	UnitLength (&vec);

	ship->rotx = 0;
//    gba_debug_32_16(0,4,abs(vec.x));
//    gba_debug_32_16(0,5,abs(vec.y));
	if (ship->type < 0)
	{
		ship->rotz = 1;
		if (((vec.x >= 0) && (vec.y >= 0)) ||
			 ((vec.x < 0) && (vec.y < 0)))
		{
			ship->rotz = -ship->rotz;
		}

		if (abs(vec.x) >= 0x40)
		{
			ship->acceleration = 0;
			ship->velocity = 1;
			return;
		}
        
        if (abs(vec.y) > 0x2)
			ship->rotx = (vec.y < 0) ? -1 : 1;

		if (abs(vec.y) >= 0x40)
		{
			 ship->acceleration = 0;
			 ship->velocity = 1;
			 return;
		}
	}

	ship->rotz = 0;

	dir = vector_dot_product (&ship->rotmat[0], &universe[1].rotmat[1]);
    dir >>= FIXEDPOINT;
    
  // 0x399 = 921
  // 0x3AB = 939
	if (abs(dir) >= 0x3f0)
	{
		ship->acceleration++;
		ship->rotz = 127;
		return;
	}

	ship->acceleration = 0;
	ship->rotz = 0;
}


/*
 * Fly a ship to the planet or to the space station and dock it.
 */

void auto_pilot_ship (Object *ship)
{
	Vertex vec;
	signed long int dist;
	signed long int dir;
	
	if ((ship->flags & FLG_FLY_TO_PLANET) ||
		((ship_count[SHIP_CORIOLIS] == 0) && (ship_count[SHIP_DODEC] == 0)))
	{
		fly_to_planet (ship);
		return;
	}

	vec.x = (ship->location.x - universe[1].location.x)>>8;	
	vec.y = (ship->location.y - universe[1].location.y)>>8;	
	vec.z = (ship->location.z - universe[1].location.z)>>8;	

	dist = gba_sqrt ((vec.x) * (vec.x) + (vec.y) * (vec.y) + (vec.z) * (vec.z));

	if (dist < 160) {
	  if (!(ship->flags & FLG_REMOVE))
		  ship->flags |= FLG_REMOVE;		// Ship has docked.
		return;
	}	
	
	UnitLength (&vec);
	
	dir = vector_dot_product (&universe[1].rotmat[2], &vec);
  
  dir >>= FIXEDPOINT;
	if (dir < 0x3FE) {
//        gba_print(0,0,"Flying to station front...");      
		fly_to_station_front (ship);
		return;
	}

	dir = vector_dot_product (&ship->rotmat[2], &vec);
    dir >>= FIXEDPOINT;
  
	if (dir < -0x3D7)
	{
	  
//        gba_print(0,0,"Flying to docking bay...");      
		fly_to_docking_bay (ship);
		return;
	}
	
//    gba_print(0,0,"Flying to station...");      
	fly_to_station (ship);
}


void engage_auto_pilot (void)
{
	if ((game_state_flag & AUTO_PILOT) || (game_state_flag & WITCHSPACE) || (game_state_flag & HYPER_READY))
		return; 

	if (!(game_draw_flag & BIG_C_DRAWN)
	&& ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
        (current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))) {
	  gfx_draw_big_c();
	}
	game_state_flag |= AUTO_PILOT;
	
    if (game_state_flag & DOCK_MUSIC)
        PlaySong(1);
}


void disengage_auto_pilot (void)
{
	if (game_state_flag & AUTO_PILOT)
	{
	  gfx_remove_big_c();
	  game_state_flag &= ~AUTO_PILOT;
	  StopSong();
	  
	  info_message("Docking Computer Off");

	}
}
