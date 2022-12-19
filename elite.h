
#ifndef ELITE_H
#define ELITE_H

#include "planet.h"
//#include "trade.h"
//
//#define GFX_COL_DARK_RED 0xBD
//#define GFX_COL_LIGHT_BLUE 0x3D
//#define GFX_COL_GREY 0x28
//#define GFX_COL_DARK_GREY 0x25
//#define GFX_COL_WHITE 0x46
//#define GFX_COL_LIGHT_GREEN 0x69
//#define GFX_COL_SHIELD_PINK 0x80
//#define GFX_COL_ORANGE 0x61
//#define GFX_COL_DARK_GREEN 0x6B
//#define GFX_COL_BROWN 0x52
//#define GFX_COL_GOLD 0xA1
//#define GFX_COL_DARK_BLUE 0x41
//#define GFX_COL_DARK_BLUE2 0xB3
//#define GFX_COL_SUN_YELLOW 0xBC
//#define GFX_COL_LASER_RED 0xBD


#define GFX_COL_DARK_RED        0xD0
#define GFX_COL_LIGHT_BLUE      0xD1
#define GFX_COL_GREY            0xD2
#define GFX_COL_DARK_GREY       0xD3
#define GFX_COL_WHITE           0xD4
#define GFX_COL_LIGHT_GREEN     0xD5
#define GFX_COL_SHIELD_PINK     0xD6
#define GFX_COL_ORANGE          0xD7
#define GFX_COL_DARK_GREEN      0xD8
#define GFX_COL_BROWN           0xD9
#define GFX_COL_GOLD            0xDA
#define GFX_COL_DARK_BLUE       0xDB
#define GFX_COL_DARK_BLUE2      0xDC
#define GFX_COL_SUN_YELLOW      0xDD
#define GFX_COL_LASER_RED       0xD0

#define GFX_COL_PLANET_GREEN        0xDE
#define GFX_COL_PLANET_MED_BLUE     0xDF
#define GFX_COL_PLANET_DARK_BLUE    0xE0
#define GFX_COL_PLANET_ORANGE       0xE1
#define GFX_COL_PLANET_PALE_BLUE    0xE2
#define GFX_COL_PLANET_PALE_VIOLET  0xE3
#define GFX_COL_PLANET_BROWN        0xE4
#define GFX_COL_PLANET_PORNY_PURPLE 0xE5
#define GFX_COL_PIN_PINK            0xE6


#define GFX_COL_MED_BLUE        0x3D

#define SCR_INTRO_ONE                1
#define SCR_INTRO_TWO                2
#define SCR_GALACTIC_CHART           3
#define SCR_SHORT_RANGE              4
#define	SCR_PLANET_DATA              5
#define SCR_MARKET_PRICES            6
#define SCR_CMDR_STATUS              7
#define SCR_FRONT_VIEW               8 
#define SCR_REAR_VIEW                9
#define SCR_LEFT_VIEW               10
#define SCR_RIGHT_VIEW              11
#define SCR_BREAK_PATTERN           12
#define SCR_INVENTORY               13
#define SCR_EQUIP_SHIP              14
#define SCR_OPTIONS                 15
#define SCR_LOAD_CMDR               16
#define SCR_SAVE_CMDR               17
#define SCR_QUIT                    18
#define SCR_EXIT                    19                    
#define SCR_GAME_OVER               20
#define SCR_SETTINGS                21
#define SCR_ESCAPE_POD              22
#define SCR_CONFIG                  23
#define SCR_MISSIONS                24
#define SCR_BREAK_PATTERN_ESC_POD   25
#define SCR_QUICK_SAVE              26


#define PULSE_LASER		0x0F
#define BEAM_LASER		0x8F
#define MILITARY_LASER	0x97
#define MINING_LASER	0x32


#define FLG_DEAD			(1)
#define	FLG_REMOVE			(2)
#define FLG_EXPLOSION		(4)
#define FLG_ANGRY			(8)
#define FLG_FIRING			(16)
#define FLG_HAS_ECM			(32)
#define FLG_HOSTILE			(64)
#define FLG_CLOAKED			(128)
#define FLG_FLY_TO_PLANET	(256)
#define FLG_FLY_TO_STATION	(512)
#define FLG_INACTIVE		(1024)
#define FLG_SLOW			(2048)
#define FLG_BOLD			(4096)
#define FLG_POLICE			(8192)

#define ELITE_SCORE(easy_mode) (easy_mode==1? 0x0A00: 0x1900)

#define rounded_div(a,b) ((a)/(b) + ((((a)%(b)<<1)>=(b))? 1: 0))

struct commander {
	unsigned char name[8];
	unsigned char mission;
	unsigned char mission_counter;
	unsigned char mission3_planet_number;
	unsigned char mission3_galaxy_number;
	unsigned char ship_x;
	unsigned char ship_y;
	struct galaxy_seed galaxy;
	unsigned long int credits;
	unsigned char fuel;
	unsigned char	galaxy_number;
	unsigned char front_laser;
	unsigned char rear_laser;
	unsigned char left_laser;
	unsigned char right_laser;
	unsigned char cargo_capacity;
	unsigned char current_cargo[17];
	unsigned long equipment; // see bit flags below
	unsigned char missiles;
	unsigned char legal_status;
	unsigned char station_stock[17];
	unsigned char market_rnd;
	unsigned short int score;
	unsigned char saved;
};

extern const struct commander default_cmdr;

#define ECM 1
#define FUEL_SCOOP 2
#define ENERGY_BOMB 4
#define ENERGY_UNIT 8
#define DOCKING_COMPUTER 16
#define GALACTIC_HYPERDRIVE 32
#define ESCAPE_POD 64
#define NAVAL_ENERGY_UNIT 128
#define ID_UNIT 256
#define CREDITS_SHOWN   512  // have we shown the credits for becoming l33t?

struct player_ship
{
	unsigned char max_speed;
	unsigned char max_roll;
	unsigned char max_climb;
	unsigned char max_fuel;
	unsigned char altitude;
	unsigned char cabtemp;
};

extern struct player_ship myship;


extern struct commander *cmdr;
extern struct commander *saved_cmdr;

//
extern struct galaxy_seed docked_planet;
//
extern struct galaxy_seed hyperspace_planet;
//
extern struct planet_data current_planet_data;
//
extern unsigned long int carry_flag;
extern unsigned long int current_screen;
extern signed short int cross_timer;
//
//extern struct ship_data *ship_list[];

extern unsigned long int game_state_flag;
/* the game state flags are:

game_over;						BIT00
docked;								BIT01
finish;								BIT02
detonate_bomb;				BIT03
witchspace;						BIT04
auto_pilot;						BIT05
instant_dock;					BIT06
*/

#define GAME_OVER            1
#define DOCKED               2
#define FINISH               4
#define DETONATE_BOMB        8
#define WITCHSPACE          16
#define AUTO_PILOT          32
#define INSTANT_DOCK        64
#define DRAW_LASERS        128
#define HYPER_READY        256
#define GAME_PAUSED        512
#define WARP_JUMP         1024 // toggle warp jump on/off
#define FIND_PLANET       2048 // toggle finding on/off
#define SOLID_SHIPS       4096 // toggle solid gfx
#define GBA_PALETTE       8192 // toggle GBA palette (gamma corrected)
#define SHOW_PROGRESS    16384 // toggle display of "Next Promotion"
#define EASY_MODE        32768 // toggle between easy rating and original rating
#define DISTANT_DOT      65536 // toggle distant objects drawn as crosses/dots
#define LUCKY_DOCK_COMP 131072 // toggle docking computer never fails
#define ESCAPING        262144 // flying to space station in escape pod
#define DOCK_MUSIC      524288 // toggle music while docking

extern unsigned long int game_draw_flag;

#define BIG_S_DRAWN 1
#define BIG_E_DRAWN 2
#define BIG_C_DRAWN 4
#define RADAR_DRAWN 8
#define MENU_DRAWN  16
#define WARP_STARS  32

// other required flags are
extern signed short int flight_speed;
extern signed short int flight_roll;
extern signed short int flight_climb;
extern unsigned short int front_shield;
extern unsigned short int aft_shield;
extern signed short int energy;
extern unsigned short int laser_temp;
extern unsigned char mcount;
extern signed char frames;

//
void initialise_cmdr(void);
void set_cmdr_default(void);
void restore_saved_commander (void);
void initialise_game(void);
void update_screen(void);
void info_message (char *message);

#endif
