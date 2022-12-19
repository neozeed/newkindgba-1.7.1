// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include <malloc.h>
#include <stdio.h>
#include "elite.h"
#include "gbalib_asm.h"
#include "space.h"
#include "shipdata.h"
#include "threed.h"
#include "scanner.h"
#include "random.h"
#include "swat.h"
#include "pilot.h"
#include "sound.h"
#include "gfx.h"
#include "menu.h"
#include "trade.h"
#include "stars.h"
#include "docked.h"
#include "planet.h"
#include "main.h"

//#include "agbprint.h"

void heapsort(unsigned char n, unsigned char * list);
void restoreHeap(unsigned char root, unsigned char last, unsigned char * list);

void move_univ_object (Object *obj);
void RollPitch (signed short int *a, signed short int *b, signed char direction);
void decrease_energy (signed short int amount);
void switch_to_view (Object *flip);
void make_station_appear (void);
void dock_player (void);
void check_docking (unsigned char i);
int is_docking (unsigned char sn);
void do_game_over (void);
void complete_hyperspace (void);
void enter_witchspace (void);
void enter_next_galaxy (void);
unsigned char rotate_byte_left (unsigned char x);

Object * universe;
unsigned short int * ship_count;
struct galaxy_seed destination_planet;
signed char hyper_countdown;
signed short int hyper_distance;
char hyper_name[16];
signed char hyper_galactic;



const static unsigned short int initial_flags[39] =
{
	0,											// NULL,
	0,											// missile 
	0,											// coriolis
	/*FLG_SLOW |*/ FLG_FLY_TO_STATION,	 // escape 
	FLG_INACTIVE,								// alloy
	FLG_INACTIVE,								// cargo
	FLG_INACTIVE,								// boulder
	FLG_INACTIVE,								// asteroid
	FLG_INACTIVE,								// rock
	FLG_FLY_TO_PLANET | FLG_SLOW,				// shuttle
	FLG_FLY_TO_PLANET | FLG_SLOW,				// transporter
	0,											// cobra3
	0,											// python
	0,											// boa
	FLG_SLOW,									// anaconda
	FLG_SLOW,									// hermit
	FLG_BOLD | FLG_POLICE,						// viper
	FLG_BOLD | FLG_ANGRY,                   // bushmaster
	FLG_BOLD | FLG_ANGRY,						// sidewinder
	FLG_BOLD | FLG_ANGRY,						// mamba
	FLG_BOLD | FLG_ANGRY,						// krait
	FLG_BOLD | FLG_ANGRY,						// adder
	FLG_BOLD | FLG_ANGRY,						// gecko
	FLG_BOLD | FLG_ANGRY,						// cobra1
	FLG_SLOW | FLG_ANGRY,						// worm
	FLG_BOLD | FLG_ANGRY,						// cobra3
	FLG_BOLD | FLG_ANGRY,						// asp2
	FLG_BOLD | FLG_ANGRY,						// python
	FLG_POLICE,									// fer_de_lance
	FLG_BOLD | FLG_ANGRY,						// moray
	FLG_BOLD | FLG_ANGRY,						// thargoid
	FLG_ANGRY,									// thargon
	FLG_ANGRY,									// constrictor
	FLG_POLICE | FLG_CLOAKED,					// cougar
	0,											// dodec
    
    FLG_SLOW ,                              // bug
    0,                                      // chameleon
    FLG_FLY_TO_PLANET,                      // shuttlev
    0,                                      // salamander
	
};

void initialise_universe(void) {
	// allocate memory for the universe. This will only ever be done once
	universe = (Object*)malloc(MAX_UNIV_OBJECTS * sizeof(Object));
    
    int i;
	for (i = 0; i < MAX_UNIV_OBJECTS; i++) {
		universe[i].type = 0;
		universe[i].flags = 0;
	}
	
	ship_count = (unsigned short int *)malloc((1+ELITE_SHIPS.nShipCount)* sizeof(unsigned short int));
	
	for (i = 0; i < ELITE_SHIPS.nShipCount; i++) {
		ship_count[i] = 0;
	}
	
}


void RollPitch (signed short int *a, signed short int *b, signed char direction) {
	signed short int fx,ux;
    
  fx = *a; // obj->rotmat[2].x
	ux = *b; // obj->rotmat[1].x

	if (direction < 0)
	{	
	    // rotate the object by 3.01 degrees.
	    // 3.01 degrees = 3.01*PI/180 = 0.05 radians  
	    // sin( theta )~= theta for small angles... 1/19 = 0.05
	    // cos(theta) ~= 0.999 for small theta
	    // the -1/512 is there to correct the fact that cos(theta) <~ theta
	    // changed to use shifts
		*a = fx - (fx >> 9) + ( (ux >>5) + (ux >>6) + (ux >>7));
		*b = ux - (ux >> 9 ) - ( (fx >>5) + (fx >>6) + (fx >>7));
	}
	else
	{
	  // rotate the object by -3 degrees...
	  *a = fx - (fx >> 9) - ( (ux >>5) + (ux >>6) + (ux >>7));
	  *b = ux - (ux >> 9 ) + ( (fx >>5) + (fx >>6) + (fx >>7));
	}
}


void dock_player (void)
{
	disengage_auto_pilot();
	
	gfx_remove_scanner();
	gfx_remove_big_s();
    gfx_remove_big_e();
    gfx_remove_big_c();
	
	if (!(game_state_flag & DOCKED))
		game_state_flag |= DOCKED;
	game_state_flag = game_state_flag & ~HYPER_READY;
	
	flight_speed = 0;
	flight_roll = 0;
	flight_climb = 0;
	front_shield = 255;
	aft_shield = 255;
	energy = 255;
	myship.altitude = 255;
	myship.cabtemp = 30;
	reset_weapons();
	menu_set_docked();
}

void update_console (void)
{
	
		
	display_speed();
	display_flight_climb();
    display_flight_roll();
	display_shields();
	display_altitude();
	display_energy();
	display_cabin_temp();
	display_laser_temp();
	display_fuel();
	display_missiles();
//	
//	if (game_state_flag & DOCKED)
//		return;
//
	update_scanner();
	update_compass();
//
	if ((ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC]) && 
	((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
     (current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW) ||
     (current_screen == SCR_ESCAPE_POD))
	&& !(game_draw_flag & MENU_DRAWN))
	 {
		gfx_draw_big_s();
//		game_draw_flag |= BIG_S_DRAWN;
	} else if (!(ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC]) && (game_draw_flag & BIG_S_DRAWN)) {
		gfx_remove_big_s();
//		game_draw_flag &= ~BIG_S_DRAWN;
	}
	
		
//
	if ((ecm_active) && !(game_draw_flag & BIG_E_DRAWN)) {
		gfx_draw_big_e();
		game_draw_flag |= BIG_E_DRAWN;
	} else if (!(ecm_active) && (game_draw_flag & BIG_E_DRAWN)) {
		gfx_remove_big_e();
		game_draw_flag &= ~BIG_E_DRAWN;
	}
	

}



/*
 * Update an objects location in the universe.
 */

void move_univ_object (Object *obj)
{
	signed long long x,y,z;
	signed long long k2;
	signed short int alpha = 0;
  signed short int beta = 0;
	signed short int rotx,rotz;
	signed long int  speed;
	
	alpha = flight_roll;
	beta = flight_climb;
	
	x = obj->location.x;
	y = obj->location.y;
	z = obj->location.z;

	if (!(obj->flags & FLG_DEAD))
	{ 
		if (obj->velocity != 0)
		{
			speed = obj->velocity;
			speed += (speed>>1); 	
			x += (obj->rotmat[2].x * speed)>>2; // >>FIXEDPOINT <<8
			y += (obj->rotmat[2].y * speed)>>2; 
			z += (obj->rotmat[2].z * speed)>>2; 
		} 
		
		if (obj->acceleration != 0)
		{
			obj->velocity += obj->acceleration;
			obj->acceleration = 0;
			if (obj->velocity > ELITE_SHIPS.pShipDataTable[obj->type].pShipData->velocity)
				obj->velocity = ELITE_SHIPS.pShipDataTable[obj->type].pShipData->velocity;
			
		}
		if ( (obj->type != SHIP_PLANET) && (obj->velocity <= 0) && 
		  (current_screen != SCR_INTRO_ONE) &&
			(current_screen != SCR_INTRO_TWO) &&
			(current_screen != SCR_GAME_OVER) &&
			(current_screen != SCR_ESCAPE_POD)&&
			(current_screen != SCR_MISSIONS) ){
	  	obj->velocity = 1;
	  }
		  
	}
	
	k2 = (y) - ((alpha * x)>>8);
	z = (z) + ((beta * k2)>>8);
	y = (k2) - ((z * beta)>>8);
	x = (x) + ((alpha * y)>>8);

	z = z - (flight_speed<<8);

	obj->location.x = x;
	obj->location.y = y;
	obj->location.z = z;	

	x = x>>16;
	y = y>>16;
	z = z>>16;
	// to prevent overflow, should we calculate the distance really roughly
	obj->distance = gba_sqrt((x)*(x) + (y)*(y) + (z)*(z))<<8;

	if (obj->type == SHIP_PLANET) {

	  beta = 0;
	}
	rotate_vec (&obj->rotmat[2], alpha, beta);
	rotate_vec (&obj->rotmat[1], alpha, beta);
	rotate_vec (&obj->rotmat[0], alpha, beta);

	if (obj->flags & FLG_DEAD)
		return;


	rotx = obj->rotx;
	rotz = obj->rotz;
	
	/* If necessary rotate the object around the X axis... */
	if (rotx != 0)
	{
		RollPitch(&obj->rotmat[2].x, &obj->rotmat[1].x, rotx);
		RollPitch(&obj->rotmat[2].y, &obj->rotmat[1].y, rotx);	
		RollPitch(&obj->rotmat[2].z, &obj->rotmat[1].z, rotx);

		if ((rotx != 127) && (rotx != -127))
			obj->rotx -= (rotx < 0) ? -1 : 1;
	}	

	
	/* If necessary rotate the object around the Z axis... */

	if (rotz != 0)
	{	
		RollPitch(&obj->rotmat[0].x, &obj->rotmat[1].x, rotz);
		RollPitch(&obj->rotmat[0].y, &obj->rotmat[1].y, rotz);	
		RollPitch(&obj->rotmat[0].z, &obj->rotmat[1].z, rotz);	

		if ((rotz != 127) && (rotz != -127))
			obj->rotz -= (rotz < 0) ? -1 : 1;
	}


	/* Orthonormalize the rotation matrix... */

	tidy_matrix (obj->rotmat);
}

	

/*
 * Update all the objects in the universe and render them.
 */

void update_universe (int plot_starfield)
{
  signed char i;
  unsigned short bounty;
  char str[80];
  Object flip;
  unsigned char order[MAX_UNIV_OBJECTS];
  unsigned char index = 0;

  for (i = 0; i < MAX_UNIV_OBJECTS; i++)
  {
    if (universe[i].type != 0)
    {
      ////////////////////////////////////////////////
      if (universe[i].flags & FLG_REMOVE)
      {
        if (universe[i].type == SHIP_VIPER)
          cmdr->legal_status |= 64;

        bounty = ELITE_SHIPS.pShipDataTable[universe[i].type].pShipData->bounty;
        if ((bounty != 0) && !(game_state_flag & WITCHSPACE)) {
          cmdr->credits += bounty;

          sprintf (str, "%d.%d Cr", (int)gba_div(cmdr->credits , 10), (int)(cmdr->credits % 10));
          info_message (str);
        }

        remove_ship (i);
        continue;
      }

      ////////////////////////////////////////////////  
      if ((game_state_flag & DETONATE_BOMB) && ((universe[i].flags & FLG_DEAD) == 0) &&
          (universe[i].type != SHIP_PLANET) && (universe[i].type != SHIP_SUN) &&
          (universe[i].type != SHIP_CONSTRICTOR) && (universe[i].type != SHIP_COUGAR) &&
          (universe[i].type != SHIP_CORIOLIS) && (universe[i].type != SHIP_DODEC))
      {
        PlaySoundFX(SND_EXPLODE);
        //				snd_play_sample (SND_EXPLODE);
        universe[i].flags |= FLG_DEAD;
      }
      ////////////////////////////////////////////////  

      if ((current_screen != SCR_INTRO_ONE) &&
          (current_screen != SCR_INTRO_TWO) &&
          (current_screen != SCR_GAME_OVER) &&
          (current_screen != SCR_ESCAPE_POD)&&
          (current_screen != SCR_MISSIONS)) {
        tactics (i);
      }
      ////////////////////////////////////////////////

      //      gba_debug_16_16(0,i+1,universe[i].type);
      //      gba_debug_16_16(5,i+1,universe[i].flags);
      move_univ_object (&universe[i]);
      flip = universe[i];
      switch_to_view (&flip);
      ////////////////////////////////////////////////
      if (universe[i].type == SHIP_PLANET) {
        if ((ship_count[SHIP_CORIOLIS] == 0) &&
            (ship_count[SHIP_DODEC] == 0) &&
            (universe[i].distance < 65535)) // was 49152
        {
          make_station_appear();
        }				

        //        draw_ship (&flip);
        order[index++] = i;
        continue;
      }
      ////////////////////////////////////////////////
      if (universe[i].type == SHIP_SUN) {
        //        draw_ship (&flip);
        order[index++] = i;
        continue;
      }
      ////////////////////////////////////////////////
      if (universe[i].distance < 0x1000) {
        signed long dis;

        // get a more accurate distance calculation...
        dis = gba_sqrt( (universe[i].location.x>>8)*(universe[i].location.x>>8) +
            (universe[i].location.y>>8)*(universe[i].location.y>>8) +
            (universe[i].location.z>>8)*(universe[i].location.z>>8));
        if (dis < 170) {
          if ((universe[i].type == SHIP_CORIOLIS) || (universe[i].type == SHIP_DODEC))
            check_docking (i);
          else
            scoop_item(i);
          continue;
        }
      }
      ////////////////////////////////////////////////
      if (universe[i].distance > 57344) {
        remove_ship (i);
        continue;
      }
      ////////////////////////////////////////////////
      //			draw_ship (&flip);
      order[index++] = i;

      universe[i].flags = flip.flags;
      universe[i].exp_seed = flip.exp_seed;
      universe[i].exp_delta = flip.exp_delta;
      //
      //			universe[i].flags &= ~FLG_FIRING;

      if (universe[i].flags & FLG_DEAD)
        continue;

      check_target (i, &flip);
      ////////////////////////////////////////////////

    }
  }

  // order contains the ships to draw, in numerical order.
  // need to sort the list by depth - i.e. furthest away to nearest
  // this stops ships far away being drawn on top of near ships, just
  // because the furthest ship was created first.
  heapsort(index, order);
  int starfield_plot_state = 0; // 0: not (to be) plotted, 1: to be plotted, 2: already plotted
  if (plot_starfield)
    update_starfield(1); // fix stars
  for (i = index-1; i >=0; i--) {
    flip = universe[order[i]];
    switch_to_view (&flip);

    if ((flip.type != SHIP_PLANET && flip.type != SHIP_SUN) && starfield_plot_state==0)
        starfield_plot_state++;
    if (starfield_plot_state == 1 && plot_starfield) {
        update_starfield(0);
        starfield_plot_state++;
    }
        
    draw_ship (&flip);
    universe[order[i]].flags = flip.flags;
    universe[order[i]].exp_seed = flip.exp_seed;
    universe[order[i]].exp_delta = flip.exp_delta;
    universe[order[i]].flags &= ~FLG_FIRING;
  }
  if (starfield_plot_state != 2 && plot_starfield)
    update_starfield(0);

  if (game_draw_flag & WARP_STARS)
    game_draw_flag &= ~WARP_STARS;

  if (game_state_flag & DETONATE_BOMB)
    game_state_flag &= ~DETONATE_BOMB;

}


/////////////////////////////////////////////////////////
void restoreHeap(unsigned char root, unsigned char last, unsigned char * list) {
  unsigned short int child = 2*root + 1;
  unsigned char next;
  // The children of the i th node are located at (2*i)+1 
  unsigned char isAHeap = 0;
  unsigned char tmp;
  while (!isAHeap && child < last) {
    // child = index of greatest child of root
    next = child +1;
    if ((next < last) && (universe[list[child]].distance < universe[list[next]].distance))
      child = next;
    
    // If either child is greater than the parent then swap the parent with the greatest child.
    // IF value of dist[root] < value of dist[child] THEN
    //  exchange list[root] and list[child]
    //  move down to next level
    if (universe[list[root]].distance < universe[list[child]].distance) {
      tmp = list[root];
      list[root] = list[child];
      list[child] = tmp;
      
      root = child;
      child = 2*root + 1;
    } else {
      isAHeap = 1;
    }
  }
}


/*
This sorts the ships by distance (which has massive rounding errors)
therefore it is far from optimal, but it works ok in practice (try shooting the
space station from behind and watch the vipers come out - then compare to a 
GBA elite build without this sorting - the station will be "see through"...)
*/
void heapsort(unsigned char n, unsigned char * list) {
  signed short i;  
  unsigned char tmp;
  // Make the heap...
  for (i= n>>1; i >= 0; i--)
    restoreHeap(i, n, list);
  
  while (n--) {
    tmp = list[0];
    list[0] = list[n];
    list[n] = tmp;
    restoreHeap(0, n, list);
  }
}
/////////////////////////////////////////////////////////


signed char add_new_ship (signed short int ship_type, 
								signed long int x, signed long int y, signed long int z, 
								Vertex *rotmat, 
								signed short int rotx, signed short int rotz)
{
	int i;

	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if (universe[i].type == 0)
		{
			universe[i].type = ship_type;
			universe[i].location.x = x;
			universe[i].location.y = y;
			universe[i].location.z = z;
			
			universe[i].distance = gba_sqrt((x>>8)*(x>>8) + (y>>8)*(y>>8) + (z>>8)*(z>>8));

			universe[i].rotmat[0].x = rotmat[0].x;
			universe[i].rotmat[0].y = rotmat[0].y;
			universe[i].rotmat[0].z = rotmat[0].z;
			
			universe[i].rotmat[1].x = rotmat[1].x;
			universe[i].rotmat[1].y = rotmat[1].y;
			universe[i].rotmat[1].z = rotmat[1].z;
			
			universe[i].rotmat[2].x = rotmat[2].x;
			universe[i].rotmat[2].y = rotmat[2].y;
			universe[i].rotmat[2].z = rotmat[2].z;
			
			
			universe[i].rotx = rotx;
			universe[i].rotz = rotz;
			
			universe[i].velocity = 0;
			universe[i].acceleration = 0;
			universe[i].bravery = 0;
			universe[i].target = 0;
			
			universe[i].flags = 0;

			if ((ship_type != SHIP_PLANET) && (ship_type != SHIP_SUN))
			{
			    universe[i].flags = initial_flags[ship_type];	
				universe[i].energy = ELITE_SHIPS.pShipDataTable[ship_type].pShipData->energy;
				universe[i].missiles = ELITE_SHIPS.pShipDataTable[ship_type].pShipData->missiles;
				ship_count[ship_type]++;
			}
			if (ship_type == SHIP_PLANET)
                universe[i].colour = planet_colour(docked_planet);
			return i;
		}
	}

	return -1;
}

void update_altitude (void)
{
	signed long int x,y,z;
	signed long int dist;
	
	myship.altitude = 255;

	if (game_state_flag & WITCHSPACE)
		return;
	
	x = universe[0].location.x>>8;
	y = universe[0].location.y>>8;
	z = universe[0].location.z>>8;
	
	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;
	if (z < 0)
		z = -z;
	
	
	
	if ((x > 65535) || (y > 65535) || (z > 65535)) {
		return;
  }
  
	x >>= 8;
	y >>= 8;
	z >>= 8;
	
	dist = (x * x) + (y * y) + (z * z);

	if (dist > 65535) {
		return;
	}
	
	
	dist -= 9472;
	
	if (dist < 1)
	{
		myship.altitude = 0;
		do_game_over ();
		return;
	}

	dist = gba_sqrt(dist);
	if (dist < 1)
	{
		myship.altitude = 0;
		do_game_over ();
		return;
	}

	myship.altitude = dist;	
}


void decrease_energy (signed short int amount)
{
  
  energy += amount;
	if (energy  <= 0) {
      do_game_over();
	}
    
}


/*
 * Deplete the shields.  Drain the energy banks if the shields fail.
 */

void damage_ship (signed short int damage, signed short int front)
{
	signed short int shield;

	if (damage <= 0)	/* sanity check */
		return;
	
	shield = front ? front_shield : aft_shield;
	
	shield -= damage;
	if (shield < 0)
	{
		decrease_energy (shield);
		shield = 0;
	}
	
	if (front)
		front_shield = shield;
	else
		aft_shield = shield;
}



void launch_player (void)
{
	Matrix rotmat;
  game_state_flag &= ~DOCKED;
	
	flight_speed = 12;
	flight_roll = -15;
	flight_climb = 0;
	cmdr->legal_status |= carrying_contraband();
	create_new_stars();
	clear_universe();
//	generate_landscape(docked_planet.a * 251 + docked_planet.b);
        set_unit_matrix(rotmat);
	
	add_new_ship (SHIP_PLANET, 0, 0, 65536<<8, rotmat, 0, 0);

	rotmat[2].x = -rotmat[2].x;
	rotmat[2].y = -rotmat[2].y;
	rotmat[2].z = -rotmat[2].z;
	add_new_station (0, 0, -256<<8, rotmat);

	current_screen = SCR_BREAK_PATTERN;
	PlaySoundFX(SND_LAUNCH);
//	snd_play_sample (SND_LAUNCH);

    last_message_out = last_message_in = 0;
    // Clear message buffer

}

void increase_flight_roll (void)
{
	if (flight_roll < myship.max_roll)
		flight_roll++;
}


void decrease_flight_roll (void)
{
	if (flight_roll > -myship.max_roll)
		flight_roll--;
}


void increase_flight_climb (void)
{
	if (flight_climb < myship.max_climb)
		flight_climb++;
}


void switch_to_view (Object *flip)
{
	signed long int ltmp;
	signed short int stmp;
	
	if ((current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_GAME_OVER))
	{
		flip->location.x = -flip->location.x;
		flip->location.z = -flip->location.z;

		flip->rotmat[0].x = -flip->rotmat[0].x;
		flip->rotmat[0].z = -flip->rotmat[0].z;

		flip->rotmat[1].x = -flip->rotmat[1].x;
		flip->rotmat[1].z = -flip->rotmat[1].z;

		flip->rotmat[2].x = -flip->rotmat[2].x;
		flip->rotmat[2].z = -flip->rotmat[2].z;
		return;
	}


	if (current_screen == SCR_LEFT_VIEW)
	{
		ltmp = flip->location.x;
		flip->location.x = flip->location.z;
		flip->location.z = -ltmp;

    
		if (flip->type < 0)
			return;
		
		stmp = flip->rotmat[0].x;
		flip->rotmat[0].x = flip->rotmat[0].z;
		flip->rotmat[0].z = -stmp;		

		stmp = flip->rotmat[1].x;
		flip->rotmat[1].x = flip->rotmat[1].z;
		flip->rotmat[1].z = -stmp;		

		stmp = flip->rotmat[2].x;
		flip->rotmat[2].x = flip->rotmat[2].z;
		flip->rotmat[2].z = -stmp;		
		return;
	}

	if (current_screen == SCR_RIGHT_VIEW)
	{
		ltmp = flip->location.x;
		flip->location.x = -flip->location.z;
		flip->location.z = ltmp;

		if (flip->type < 0)
			return;
		
		stmp = flip->rotmat[0].x;
		flip->rotmat[0].x = -flip->rotmat[0].z;
		flip->rotmat[0].z = stmp;		

		stmp = flip->rotmat[1].x;
		flip->rotmat[1].x = -flip->rotmat[1].z;
		flip->rotmat[1].z = stmp;		

		stmp = flip->rotmat[2].x;
		flip->rotmat[2].x = -flip->rotmat[2].z;
		flip->rotmat[2].z = stmp;

	}
}

void decrease_flight_climb (void)
{
	if (flight_climb > -myship.max_climb)
		flight_climb--;
}

void display_front_view(void) {
  if (!(game_state_flag & DOCKED)) {
    if (current_screen != SCR_FRONT_VIEW && 
        current_screen != SCR_ESCAPE_POD)
  	{
  		current_screen = (game_state_flag & ESCAPING)? SCR_ESCAPE_POD:SCR_FRONT_VIEW;
  		flip_stars();
  	}
  }
}

void display_rear_view(void) {
  if (!(game_state_flag & DOCKED)) {
    if (current_screen != SCR_REAR_VIEW)
  	{
  		current_screen = SCR_REAR_VIEW;
  		flip_stars();
  	}
  }
}

void display_left_view(void) {
  if (!(game_state_flag & DOCKED)) {
    if (current_screen != SCR_LEFT_VIEW)
  	{
  		current_screen = SCR_LEFT_VIEW;
  		flip_stars();
  	}
  }
}

void display_right_view(void) {
  if (!(game_state_flag & DOCKED)) {
    if (current_screen != SCR_RIGHT_VIEW)
  	{
  		current_screen = SCR_RIGHT_VIEW;
  		flip_stars();
  	}
  }
}

void make_station_appear (void)
{
	signed long int px,py,pz;
	signed long int sx,sy,sz;
	Vertex vec;
	Matrix rotmat;
	
	px = universe[0].location.x>>8;
	py = universe[0].location.y>>8;
	pz = universe[0].location.z>>8;
    
    // 32767 = 0x7FFF = 15 bits
    // 16384 = 0x4000 = 1<<14 
	vec.x = GetRand(32767) - 16384;	
	vec.y = GetRand(32767) - 16384;	
	vec.z = GetRand(32767);	

	UnitLength(&vec);
    
    // 65792 = 0x10100 = 1<<16 + 1<<8
    // i.e. increase the value by 50% and shift the number up by 16
    // shift down by fixedpoint because the UnitLength routine returns 
    // the normalised values in fixedpoint form
	sx = px - ((vec.x * 65792)>>FIXEDPOINT);
	sy = py - ((vec.y * 65792)>>FIXEDPOINT);
	sz = pz - ((vec.z * 65792)>>FIXEDPOINT);

	// shockingly, normalising the vector gave us a station 
	// pointing towards the planet. I am quite frankly amazed.
	rotmat[0].x = 1<<FIXEDPOINT;
	rotmat[0].y = 0;
	rotmat[0].z = 0;

	rotmat[1].x = vec.x;
	rotmat[1].y = vec.z;
	rotmat[1].z = -vec.y;
	
	rotmat[2].x = vec.x;
	rotmat[2].y = vec.y;
	rotmat[2].z = vec.z;

	tidy_matrix (rotmat);

    add_new_station (sx<<8, sy<<8, sz<<8, rotmat);
}

void check_docking (unsigned char i)
{
    
	if (is_docking(i))
	{
    dock_player();
    PlaySoundFX(SND_DOCK);
		current_screen = SCR_BREAK_PATTERN;
		return;
	}
	
	if (flight_speed >= 5)
	{
		do_game_over();
		return;
	}

	flight_speed = 1;
	damage_ship (5, universe[i].location.z > 0);
	PlaySoundFX(SND_CRASH);
//	snd_play_sample (SND_CRASH);
}

/*
 * Check if we are correctly aligned to dock.
 */

int is_docking (unsigned char sn)
{
	Vertex vec;
	signed long fz;
	signed long ux;
    
	if (((game_state_flag & AUTO_PILOT) && (game_state_flag & LUCKY_DOCK_COMP))
	    // (Don't want it to kill anyone!) Why not!
        || (game_state_flag & ESCAPING)) 
        // OK, we do not want to kill anyone when docking with the escape pod
		return 1;
	
	fz = universe[sn].rotmat[2].z;

	if ((-0x3f6) < fz ) {  // seems to be more realistic TM

		return 0;
	}
		
	vec.x = universe[sn].location.x>>8;
	vec.y = universe[sn].location.y>>8;
	vec.z = universe[sn].location.z>>8;
	
	UnitLength(&vec);
	
  if (vec.z < 0x3B5 ){// 0.927 = 0x3B5
		return 0;
  }
		
	
	ux = universe[sn].rotmat[1].x;
	if (ux < 0)
		ux = -ux;

	if (ux < 0x35C) {
		return 0;
	}
	return 1;
}

void do_game_over (void)
{
	if (!(game_state_flag & GAME_OVER))
	  game_state_flag |= GAME_OVER;
}

void engage_docking_computer (void)
{
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
	{
    PlaySoundFX(SND_DOCK);
		dock_player();
		current_screen = SCR_BREAK_PATTERN;
	}
}

void start_hyperspace (void)
{
	if (game_state_flag & HYPER_READY || game_state_flag & DOCKED)
		return;
		
	hyper_distance = calc_distance_to_planet (&docked_planet, &hyperspace_planet);

	if ((hyper_distance == 0) || (hyper_distance > cmdr->fuel))
		return;

	destination_planet = hyperspace_planet;
	name_planet (hyper_name, destination_planet);
	capitalise_name (hyper_name);
	
	game_state_flag |= HYPER_READY;
	hyper_countdown = 15;
	hyper_galactic = 0;

	disengage_auto_pilot();
}

void start_galactic_hyperspace (void)
{
  if (game_state_flag & DOCKED)
    return;
	if (game_state_flag & HYPER_READY)
		return;

	if (!(cmdr->equipment & GALACTIC_HYPERDRIVE)) {
		return;
  }
	game_state_flag |= HYPER_READY;
	hyper_countdown = 10;
	hyper_galactic = 1;
	disengage_auto_pilot();
}


void display_hyper_status (void)
{
	char str[30];

	sprintf (str, "%d", hyper_countdown);	
  gba_setcolour(GFX_COL_WHITE);
  
  if (!((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))){
		  // print unmasked on every screen except the view ones
		  gba_print (1, 1, str);  	
  }
		
	if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
		(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
	{
	  gba_print_masked(1, 2, str);  	
		if (hyper_galactic)
		{
		  
			gfx_centre_text_masked(11, "Galactic Hyperspace");
		}
		else
		{
			sprintf (str, "Hyperspace - %s", hyper_name);
			gfx_centre_text_masked(11, str);
		} 	
	}
}

void countdown_hyperspace (void)
{
	if (hyper_countdown == 0)
	{
		complete_hyperspace();
		return;
	}

	hyper_countdown--;
}


void complete_hyperspace (void)
{
	Matrix rotmat;
	int px,py,pz;
	
	// remove the menu when we hyperspace
	if (game_draw_flag & MENU_DRAWN) {
	  gfx_remove_menu();
	  game_draw_flag &= ~MENU_DRAWN;
	}
	
	if (game_state_flag & HYPER_READY)
	  game_state_flag &= ~HYPER_READY;
	if (game_state_flag & WITCHSPACE)
	  game_state_flag &= ~WITCHSPACE;
	
	if (hyper_galactic)
	{
		cmdr->equipment &= ~GALACTIC_HYPERDRIVE;
		enter_next_galaxy();
		cmdr->legal_status = 0;
	}
	else
	{
		cmdr->fuel -= hyper_distance;
		cmdr->legal_status >>= 1;

		if ((rand255() > 253) || (flight_climb == myship.max_climb))
		{
			enter_witchspace();
			return;
		}

		docked_planet = destination_planet; 
	}

	cmdr->market_rnd = rand255();
	generate_planet_data (&current_planet_data, docked_planet);
	generate_stock_market ();
	
	flight_speed = 12;
	flight_roll = 0;
	flight_climb = 0;
	create_new_stars();
	clear_universe();

//	generate_landscape(docked_planet.a * 251 + docked_planet.b);
	set_unit_matrix (rotmat);
	

	pz = (((docked_planet.b) & 7) + 7) >>1;
	px = pz >>1;
	py = px;

  // this <<16 is in TNK...
	px <<= 16;
	py <<= 16;
	pz <<= 16;
	
	if ((docked_planet.b & 1) == 0)
	{
		px = -px;
		py = -py;
	}

	add_new_ship (SHIP_PLANET, px<<8, py<<8, pz<<8, rotmat, 0, 0);

	pz = -(((docked_planet.d & 7) | 1) << 16);
	px = ((docked_planet.f & 3) << 16) | ((docked_planet.f & 3) << 8);

	add_new_ship (SHIP_SUN, px<<8, py<<8, pz<<8, rotmat, 0, 0);

	current_screen = SCR_BREAK_PATTERN;
	PlaySoundFX(SND_HYPERSPACE);
	
}


void jump_warp (void)
{
	unsigned char i;
	signed char type;
	signed long int jump;
	
	// switch off warp jump for now, turn it on at the end
	game_state_flag &= ~WARP_JUMP;
	
	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		type = universe[i].type;
		
		if ((type > 0) && (type != SHIP_ASTEROID) && (type != SHIP_CARGO) &&
			(type != SHIP_ALLOY) && (type != SHIP_ROCK) &&
			(type != SHIP_BOULDER) && (type != SHIP_ESCAPE_CAPSULE))
		{
			info_message ("Mass-Locked");
			return;
		}
	}

	if ((universe[0].distance < 75001) || (universe[1].distance < 75001))
	{
		info_message ("Mass-Locked");
		return;
	}


	if (universe[0].distance < universe[1].distance)
		jump = universe[0].distance - 75000;
	else
		jump = universe[1].distance - 75000;	

	if (jump > 1024)
		jump = 1024;
	
	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if (universe[i].type != 0)
			universe[i].location.z -= (jump<<8);
	}
    
  game_draw_flag |= WARP_STARS;
	mcount &= 63;
	swat_flag &= ~IN_BATTLE;
  game_state_flag |= WARP_JUMP;
}



/*
 * Regenerate the shields and the energy banks.
 */

void regenerate_shields (void)
{
	if (energy > 127)
	{
		if (front_shield < 255)
		{
			front_shield++;
			energy--;
		}
	
		if (aft_shield < 255)
		{
			aft_shield++;
			energy--;
		}
	}
		
	energy++;
	if (cmdr->equipment & NAVAL_ENERGY_UNIT)
	  energy +=2;
	else if (cmdr->equipment & ENERGY_UNIT)
	  energy ++;
	  
	if (energy > 255)
		energy = 255;
}


void update_cabin_temp (void)
{
	signed long int x,y,z;
	signed long int dist;
	
	myship.cabtemp = 30;

	if (game_state_flag & WITCHSPACE)
		return;
	
	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
		return;
	
	x = abs((int)universe[1].location.x>>8);
	y = abs((int)universe[1].location.y>>8);
	z = abs((int)universe[1].location.z>>8);
	
	if ((x > 65535) || (y > 65535) || (z > 65535))
		return;

	x >>= 8;
	y >>= 8;
	z >>= 8;
	
	dist = ((x * x) + (y * y) + (z * z)) >> 8;

	if (dist > 255)
		return;

  	dist ^=  255;


	if ((dist+30) > 255)
	{
		myship.cabtemp = 255;
		do_game_over ();
		return;
	}
	myship.cabtemp = dist + 30;
	
	if ((myship.cabtemp < 224) || (!(cmdr->equipment & FUEL_SCOOP)))
		return;

	cmdr->fuel += flight_speed >> 2;
	if (cmdr->fuel > myship.max_fuel)
		cmdr->fuel = myship.max_fuel;

	info_message ("Fuel Scoops On");	
}



void docking_computer_selected(void) {
  // if not docked, have docking computer and are near the station...
  if ((!(game_state_flag & DOCKED)) && (cmdr->equipment & DOCKING_COMPUTER)
        && (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC]) )
	{
		if (game_state_flag & INSTANT_DOCK)
			engage_docking_computer();
		else
			engage_auto_pilot();
	}
}

void ecm_selected(void) {
  if (!(game_state_flag & DOCKED) && cmdr->equipment & ECM)
    activate_ecm(1);
}

void escape_capsule_selected(void) {
  if (!(game_state_flag & DOCKED) && (cmdr->equipment & ESCAPE_POD)) {
    run_escape_sequence(0);
    
  }
  
}


void energy_bomb_selected(void) {
  if (!(game_state_flag & DOCKED) && (cmdr->equipment & ENERGY_BOMB))
  {
    if (!(game_state_flag & DETONATE_BOMB))
      game_state_flag |= DETONATE_BOMB;
    cmdr->equipment &= ~ENERGY_BOMB;
//    PlaySoundFX(SND_EXPLODE);
  }
}

void enter_next_galaxy (void)
{
	cmdr->galaxy_number++;
	cmdr->galaxy_number &= 7;
	
	cmdr->galaxy.a = rotate_byte_left (cmdr->galaxy.a);
	cmdr->galaxy.b = rotate_byte_left (cmdr->galaxy.b);
	cmdr->galaxy.c = rotate_byte_left (cmdr->galaxy.c);
	cmdr->galaxy.d = rotate_byte_left (cmdr->galaxy.d);
	cmdr->galaxy.e = rotate_byte_left (cmdr->galaxy.e);
	cmdr->galaxy.f = rotate_byte_left (cmdr->galaxy.f);

	docked_planet = find_planet (0x60, 0x60);
	hyperspace_planet = docked_planet;
}

unsigned char rotate_byte_left (unsigned char x)
{
	return ((x << 1) | (x >> 7)) & 255;
}


void enter_witchspace (void)
{
	unsigned char i;
	unsigned char nthg;
	
	game_state_flag |=  WITCHSPACE;
	docked_planet.b ^= 31;
	swat_flag |= IN_BATTLE;
  
  
	flight_speed = 12;
	flight_roll = 0;
	flight_climb = 0;
	create_new_stars();
	clear_universe();

	nthg = (GetRand(65535) & 3) + 1;

	for (i = 0; i < nthg; i++)
		create_thargoid();	

	current_screen = SCR_BREAK_PATTERN;
	PlaySoundFX (SND_HYPERSPACE);
}


