#include <malloc.h>
#include "elite.h"
#include "planet.h"
#include "trade.h"
#include "main.h"
#include "options.h"
#include "stars.h"
#include "swat.h"
#include "missions.h"

struct player_ship myship;

struct galaxy_seed docked_planet;

struct galaxy_seed hyperspace_planet;

struct planet_data current_planet_data;

struct commander *cmdr;
struct commander *saved_cmdr;

unsigned long int carry_flag;
unsigned long int current_screen;
signed char frames;
unsigned long int game_state_flag;
unsigned long int game_draw_flag;

signed short int flight_speed;
signed short int flight_roll;
signed short int flight_climb;
unsigned short int front_shield;
unsigned short int aft_shield;
signed short int energy;
unsigned short int laser_temp;
unsigned char mcount;
signed short int cross_timer;

//const static int start_stock[] = {0x10, 0x0F, 0x11, 0x00, 0x03, 0x1C, 0x0E, 0x00,
//																	0x00, 0x0A, 0x00, 0x11,	0x3A, 0x07, 0x09, 0x08, 0x00};/* Station Stock	*/

// stored in ROM, need to copy to struct in RAM when needed
const struct commander default_cmdr =
{
	"JAMESON",									/* Name 			*/
	0,											/* Mission Number 	*/
	0,											/* Mission Counter 	*/
	3,                                          /* Mission 3 Galaxy Number */
	248,                                        /* Mission 3 Planet Number */
	0x14,0xAD,									/* Ship X,Y			*/
	{0x4a, 0x5a, 0x48, 0x02, 0x53, 0xb7},		/* Galaxy Seed		*/
	1000,										/* Credits * 10		*/
	70,											/* Fuel	* 10		*/
	0,											/* Galaxy - 1		*/
	PULSE_LASER,								/* Front Laser		*/
	0,											/* Rear Laser		*/
	0,											/* Left Laser		*/
	0,											/* Right Laser		*/
	20,											/* Cargo Capacity	*/
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},		/* Current Cargo	*/
	0, //ECM|FUEL_SCOOP|ENERGY_BOMB|ENERGY_UNIT|DOCKING_COMPUTER|GALACTIC_HYPERDRIVE|ESCAPE_POD,  	 /* equipment */
	3,											/* No. of Missiles	*/
	0,											/* Legal Status		*/
	{0x10, 0x0F, 0x11, 0x00, 0x03, 0x1C,		/* Station Stock	*/
	 0x0E, 0x00, 0x00, 0x0A, 0x00, 0x11,
	 0x3A, 0x07, 0x09, 0x08, 0x00},
	0,											/* Fluctuation		*/
	0,											/* Score			*/
	0x80										/* Saved			*/
};

void initialise_cmdr(void) {
	// called just once
	// alloc memory for the commander structures; they don't need to be in iwram
	// as they aren't called much at critical times
	cmdr = (struct commander *)malloc(sizeof(struct commander));
	saved_cmdr = (struct commander *)malloc(sizeof(struct commander));
    //sprintf(debugstring, "%x", cmdr);

    set_cmdr_default();
}

void set_cmdr_default(void) {
	// init saved with Jameson
	*saved_cmdr = default_cmdr;
	
	// set the clock to 0
	subseconds = 0; 
    seconds = 0;    
    minutes = 0;    
    hours = 0;   
    
    zoom = 0;  

	
	// set the controls to the defaults
    set_default_controls(OPTION_R_DEF, OPTION_L_DEF, OPTION_B_DEF,OPTION_S_DEF);	

    // set other options to defaults
    set_default_options();
	
}


void initialise_game(void) {
	current_screen = SCR_INTRO_ONE;
	laser_temp = 0;
	restore_saved_commander();

	game_draw_flag = 0;
	flight_speed = 1;
	flight_roll = 0;
	flight_climb = 0;
	game_state_flag |= DOCKED;
	front_shield = 255;
	aft_shield = 255;
	energy = 255;
	mcount = 0;
	cross_timer = 0;
	zoom = 0;
	last_message_in = last_message_out = 0;
	enemy_hit = 0;
	
  game_state_flag &= ~DRAW_LASERS;
  game_state_flag &= ~HYPER_READY;
  game_state_flag &= ~DETONATE_BOMB;
  game_state_flag &= ~WITCHSPACE;
  game_state_flag &= ~GAME_PAUSED;
  game_state_flag &= ~AUTO_PILOT;    
  game_state_flag &= ~GAME_OVER;
  game_state_flag &= ~FINISH;

	create_new_stars();
//    sprintf(debugstring, "%x %x", universe, cmdr);
	clear_universe();
//    sprintf(debugstring, "%x %x %x %x %x %x", 
//        cmdr->galaxy.a, cmdr->galaxy.b, cmdr->galaxy.c,
//        cmdr->galaxy.d, cmdr->galaxy.e, cmdr->galaxy.f);


	myship.max_speed = 32;		/* 0.27 Light Mach is 40 - bit slow here 0.21 LM that's so the speed dial bar draws ok */
	myship.max_roll = 31;
	myship.max_climb = 8;		/* CF 8 */
	myship.max_fuel = 70;		/* 7.0 Light Years */

}




void restore_saved_commander (void)
{
    struct galaxy_seed planet;


    *cmdr = *saved_cmdr;
	docked_planet = find_planet (cmdr->ship_x, cmdr->ship_y);
	hyperspace_planet = docked_planet;

	generate_planet_data (&current_planet_data, docked_planet);
	generate_stock_market ();
	set_stock_quantities (cmdr->station_stock);
		
	find_planet_seed(&planet, cmdr->mission3_galaxy_number, cmdr->mission3_planet_number);
	name_planet(mission3_planet_name, planet);
	capitalise_name(mission3_planet_name);
}


