
// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "docked.h"
#include "planet.h"
#include "elite.h"
#include "space.h"
#include "shipdata.h"
#include "trade.h"
#include "gfx.h"
#include "sound.h"
#include "gbalib_asm.h"

// #define EQUIP_START_Y	12 // TM
#define EQUIP_START_Y	13 // TM
#define EQUIP_START_X	1
#define EQUIP_MAX_Y		19
#define EQUIP_WIDTH		14
#define Y_INC			1

#define NO_OF_RANKS	11

char find_name[20];
char find_char[3];
signed char hilite_find_letter;
void finish_find(void);

void highlight_stock (int i);
void display_stock_price (int i);

const char *laser_type (int strength);
unsigned char equip_present (int type);
void display_equip_price (int i);
void collapse_equip_list (void);
void select_next_equip (void);
void select_previous_equip (void);
unsigned char check_laser_purchase(unsigned char dir);

void draw_fuel_limit_circle (unsigned char cx, unsigned char cy);
unsigned short calc_distance_to_planet (struct galaxy_seed *from_planet, struct galaxy_seed *to_planet);
void show_distance (unsigned char ypos, struct galaxy_seed *from_planet, struct galaxy_seed *to_planet);


struct rank
{
	int score[2];
	char *title;
};

const struct rank rating[NO_OF_RANKS] =
{
	{{0x0000, 0x0000}, "Harmless"},
	{{0x0008, 0x0008}, "Mostly Harmless"},
	{{0x0010, 0x0010}, "Poor"},
	{{0x0020, 0x0020}, "Average"},
	{{0x0040, 0x0040}, "Above Average"},
	{{0x0080, 0x0080}, "Competent"},
	{{0x0100, 0x0200}, "Dangerous"},
	{{0x0200, 0xffff}, "Very Dangerous"},
	{{0x0400, 0x0a00}, "Deadly"},
	{{0x0800, 0xffff}, "Very Deadly"},
	{{ELITE_SCORE(0), ELITE_SCORE(1)}, "--- E L I T E ---"}
};


const static char *condition_txt[] =
{
	"Docked",
	"Green",
	"Yellow",
	"Red"
};

const static char *unit_name[] = {"t", "kg", "g"};

char planet_name_for_display[19];
signed short light_years_for_display;

signed short int hilite_item;

int cross_x = 0;
int cross_y = 0;

/*********** COMMANDER STATUS *********************/
void display_commander_status (void)
{
  char planet_name[16];
	char str[100];
	int i;
	int x,y;
	unsigned char condition;
	unsigned char type;
	int current_rank=0; // TM
	int difficulty;
	
  if (game_draw_flag & MENU_DRAWN) {
    hilite_item = 0;
  }	
  
  if (game_draw_flag & RADAR_DRAWN) {
    remove_all_scanner_gfx();
  }
  
	current_screen = SCR_CMDR_STATUS;

	gba_clsUnroll();

	sprintf (str, "COMMANDER %s", cmdr->name);
	
	gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text(0, str);
//	gba_print(6,0, str);
	gba_scanline (2, 160-9, 236);
		
//  Test missions
//	sprintf(str, "%d, %x", cmdr->mission, cmdr->score);
//	gba_print(0,0, str);
	
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,2, "Present System:");
	
	if (!(game_state_flag & WITCHSPACE))
	{
		name_planet (planet_name, docked_planet);
		capitalise_name (planet_name);
		sprintf (str, "%s", planet_name);
//		sprintf (str, "%s (%d)", planet_name, find_planet_number(docked_planet));
		gba_setcolour(GFX_COL_WHITE);
		gba_print(17,2, str);
	}

	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,3, "H'space System:");
	name_planet (planet_name, hyperspace_planet);
	capitalise_name (planet_name);
	sprintf (str, "%s", planet_name);
	gba_setcolour(GFX_COL_WHITE);
	gba_print(17,3, str);

	if (game_state_flag & DOCKED) {
		condition = 0;
		gba_setcolour(GFX_COL_WHITE);
	}
	else
	{
		condition = 1;
		gba_setcolour(GFX_COL_LIGHT_GREEN);
		for (i = 0; i < MAX_UNIV_OBJECTS; i++)
		{
			type = universe[i].type;
		
			if ((type == SHIP_MISSILE) ||
				((type > SHIP_ROCK) && (type < SHIP_DODEC)))
			{
				condition = 2;
				gba_setcolour(GFX_COL_GOLD);
				break;
			}
		}
 
		if ((condition == 2) && (energy < 128)) {
			condition = 3;
			gba_setcolour(GFX_COL_DARK_RED);
		}
	}
	
	gba_print(17,4, condition_txt[condition]);
	
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,4, "Condition     :");
	

	sprintf (str, "%d.%d Light Years", (int)gba_div(cmdr->fuel , 10), (int)(cmdr->fuel % 10));
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,5, "Fuel:");
	gba_setcolour(GFX_COL_WHITE);
	gba_print(7,5, str);

	sprintf (str, "%d.%d Cr", (int)(cmdr->credits / 10), (int)(cmdr->credits % 10));
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,6, "Cash:");
	gba_setcolour(GFX_COL_WHITE);
	gba_print(7,6, str);

	if (cmdr->legal_status == 0)
		strcpy (str, "Clean");
	else
		strcpy (str, cmdr->legal_status > 50 ? "Fugitive" : "Offender");
	
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,7, "Legal Status:");
	gba_setcolour(GFX_COL_WHITE);
	gba_print(15,7, str);

    difficulty = (game_state_flag & EASY_MODE)? 0: 1;

	for (i = 0; i < NO_OF_RANKS; i++) {
		if (cmdr->score >= rating[i].score[difficulty]) {
			strcpy (str, rating[i].title);
//			sprintf(str, "%s, Miss.: %d", rating[i].title, cmdr->mission);
			current_rank = i; // TM
//			break;
		}
	}
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_print(1,8, "Rating:");
	gba_setcolour(GFX_COL_WHITE);
	gba_print(9,8, str);
	
	
	// debug the score
//	gba_setcolour(GFX_COL_DARK_RED);	
//	sprintf (str, "S:%d", cmdr->score);
//	gba_print(20,8, str);

// TM
    if ((current_rank >= 3) && (game_state_flag & SHOW_PROGRESS)) { // Next Promotion not shown below AVERAGE (would be ridiculous)
      if (current_rank < NO_OF_RANKS-1) {
        sprintf(str, "%d%%", 
                100 * (cmdr->score - rating[current_rank].score[difficulty])
                / (rating[current_rank+1].score[difficulty] - rating[current_rank].score[difficulty]));
      }
      else {
        strcpy(str, "-");
      }
      gba_setcolour(GFX_COL_LIGHT_GREEN);
	  gba_print(1,9, "Next Promotion:");
	  gba_setcolour(GFX_COL_WHITE);
	  gba_print(17,9, str);
    }
// /TM
	
	gba_setcolour(GFX_COL_LIGHT_GREEN);
//	gba_print(1,9, "Missiles:");  // TM
	gba_print(1,10, "Missiles:"); // TM
	sprintf (str, "%d", cmdr->missiles);
	gba_setcolour(GFX_COL_WHITE);
//	gba_print(11,9, str); // TM
	gba_print(11,10, str); // TM
	
	gba_setcolour(GFX_COL_GOLD);
//	gba_print(1,11, "EQUIPMENT:"); // TM
	gba_print(1,12, "EQUIPMENT:"); // TM

	x = EQUIP_START_X;
	y = EQUIP_START_Y;
	gba_setcolour(GFX_COL_WHITE);
	
	if (cmdr->cargo_capacity > 20)
	{
		gba_print(x, y, "Cargo Bay+");
		y += Y_INC;
	}
	
	if (cmdr->equipment & ESCAPE_POD)
	{
		gba_print (x, y, "Escape Pod");
		y += Y_INC;
	}
	
	if (cmdr->equipment & FUEL_SCOOP)
	{
		gba_print (x, y, "Fuel Scoops");
		y += Y_INC;
	}

	if (cmdr->equipment & ECM)
	{
		gba_print (x, y, "E.C.M.");
		y += Y_INC;
	}

	if (cmdr->equipment & ENERGY_BOMB)
	{
		gba_print (x, y, "Energy Bomb");
		y += Y_INC;
		
	}

	if (cmdr->equipment & DOCKING_COMPUTER)
	{
		gba_print (x, y, "Docking Comp.");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	
	if (cmdr->equipment & ID_UNIT)
	{
		gba_print (x, y, "ID Computer");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	
	if (cmdr->equipment & GALACTIC_HYPERDRIVE)
	{
		gba_print (x, y, "G. Hyperdrive");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if ((cmdr->equipment & ENERGY_UNIT) || (cmdr->equipment & NAVAL_ENERGY_UNIT))
	{
	  if (cmdr->equipment & NAVAL_ENERGY_UNIT)
		  gba_print (x, y,"Naval NRG Unit");
		else
		  gba_print (x, y,"Extra NRG Unit");
//				  (cmdr->equipment & NAVAL_ENERGY_UNIT) ? "Extra Energy Unit" :"Naval Energy Unit");
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr->front_laser)
	{
		sprintf (str, "F %s Laser", laser_type(cmdr->front_laser));
		gba_print (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}
	
	if (cmdr->rear_laser)
	{
		sprintf (str, "Rr %s Laser", laser_type(cmdr->rear_laser));
		gba_print (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr->left_laser)
	{
		sprintf (str, "L %s Laser", laser_type(cmdr->left_laser));
		gba_print (x, y, str);
		y += Y_INC;
		if (y > EQUIP_MAX_Y)
		{
			y = EQUIP_START_Y;
			x += EQUIP_WIDTH;
		}
	}

	if (cmdr->right_laser)
	{
		sprintf (str, "R %s Laser", laser_type(cmdr->right_laser));
		gba_print (x, y, str);
	}
}

const static char *laser_name[5] = {"Pulse", "Beam", "M'tary", "Mining", "Custom"};

const char *laser_type (int strength)
{
	switch (strength)
	{
		case PULSE_LASER:
			return laser_name[0];

		case BEAM_LASER:
			return laser_name[1];
		
		case MILITARY_LASER:
			return laser_name[2];
		
		case MINING_LASER:
			return laser_name[3];
	}	

	return laser_name[4];
}

/*********** BUY AND SELL *********************/
void display_market_prices (void)
{
  char str[100];
  char planet_name[16];
  int i;
  short int cargo_held;

  current_screen = SCR_MARKET_PRICES;
  if (game_draw_flag & RADAR_DRAWN) {
    remove_all_scanner_gfx();
  }
  gba_clsUnroll();
  
  // show tons left
  cargo_held = total_cargo();
  
  name_planet (planet_name, docked_planet);
  sprintf (str, "%s MARKET PRICES", planet_name);
  
  
  gba_setcolour(GFX_COL_GOLD);
  gfx_centre_text(0, str);
//	gba_scanline (2, 160-9, 236);
  	
  gba_setcolour(GFX_COL_LIGHT_BLUE);

  gba_print(1,1,"PRODUCT");
  gba_print(13,1,"PRICE");
  gba_print(19,1,"STOCK");
  gba_print(25,1,"CARGO");

	for (i = 0; i < 17; i++)
	{
		display_stock_price (i);
	}
	
  sprintf (str, "Cash: %d.%d Cr", (int)gba_div(cmdr->credits , 10), (int)(cmdr->credits % 10));
  gba_setcolour(GFX_COL_GOLD);
  gba_print (1, 19, str);

  sprintf(str, "%2d/%dt", cargo_held,cmdr->cargo_capacity);
  gba_print (24, 19, str);

}

void display_stock_price (int i)
{
	int y;
	char str[100];

	y = 2+i;
	if ((game_state_flag & DOCKED) && (hilite_item == i)) {
	  gba_setcolour(GFX_COL_LIGHT_GREEN);
	  //gfx_draw_spot(4, 2+y*8);
	} else {
	  gba_setcolour(GFX_COL_WHITE);
	}
  	
  
	gba_print (1,y, stock_market[i].name);

  // longest name is 12 for "Radioactives"
//  gba_print (14, y, unit_name[stock_market[i].units]);
  
	sprintf (str, "%3.0d.%d", (int)gba_div(stock_market[i].current_price , 10),
             (int)(stock_market[i].current_price % 10));
	gba_print (13, y, str);
  
  // 100.0
  
	if (stock_market[i].current_quantity > 0)
		sprintf (str, "%2.0d%s", stock_market[i].current_quantity,
							  unit_name[stock_market[i].units]);
	else
		strcpy (str, " -");

  gba_print(20,y,str);
	if (cmdr->current_cargo[i] > 0)
		sprintf (str, "%2.0d%s", cmdr->current_cargo[i],
							  unit_name[stock_market[i].units]);
	else
		strcpy (str, " -");
// 100kg
//	gfx_display_text (444, y, str);

  gba_print(26,y,str);
  
  
}

void select_previous_stock (void)
{
	if (!(game_state_flag & DOCKED) || hilite_item == 0)
		return;
  hilite_item--;
}

void select_next_stock (void)
{
	if (!(game_state_flag & DOCKED) || hilite_item == 16)
		return;
  hilite_item ++;
}

void buy_stock (void)
{
	struct stock_item *item;
	unsigned short int cargo_held;
	
	if (!(game_state_flag & DOCKED))
		return;

	item = &stock_market[hilite_item];
		
	if ((item->current_quantity == 0) ||
	    (cmdr->credits < item->current_price))
		return;

	cargo_held = total_cargo();
	
	if ((item->units == TONNES) &&
		(cargo_held == cmdr->cargo_capacity))
		return;
	
	cmdr->current_cargo[hilite_item]++;
	item->current_quantity--;
	cmdr->credits -= item->current_price;	
  PlaySoundFX(SND_SELECT);
//	highlight_stock (hilite_item);
}


void sell_stock (void)
{
	struct stock_item *item;
	
	if (!(game_state_flag & DOCKED) || (cmdr->current_cargo[hilite_item] == 0))
		return;

	item = &stock_market[hilite_item];

	cmdr->current_cargo[hilite_item]--;
	item->current_quantity++;
	cmdr->credits += item->current_price;	

  PlaySoundFX(SND_CANCEL);
//	highlight_stock (hilite_item);
}

/************ INVENTORY **************************/

void display_inventory (void)
{
	int i;
	int y;
	char str[80];
	
	current_screen = SCR_INVENTORY;
	
	if (game_draw_flag & RADAR_DRAWN) {
      remove_all_scanner_gfx();
    }
	
	gba_clsUnroll();
	
	gba_setcolour(GFX_COL_GOLD);
  gba_print(10,0,"INVENTORY");
	gba_scanline (2, 160-9, 236);
  
  sprintf (str, "%d.%d Light Years", (int)gba_div(cmdr->fuel , 10), (int)(cmdr->fuel % 10));
  gba_setcolour(GFX_COL_LIGHT_GREEN);
  gba_print(0,2,"Fuel:");
  gba_setcolour(GFX_COL_WHITE);
  gba_print(6,2,str);


  sprintf (str, "%d.%d Cr", (int)gba_div(cmdr->credits , 10), (int)(cmdr->credits % 10));
  gba_setcolour(GFX_COL_LIGHT_GREEN);
  gba_print(0,3,"Cash:");
  gba_setcolour(GFX_COL_WHITE);
  gba_print(6,3,str);

	y = 4;
	// gba_setcolour(GFX_COL_WHITE);
	for (i = 0; i < 17 && y < 20; i++)
	{
		if (cmdr->current_cargo[i] > 0)
		{
			gba_print (1, y, stock_market[i].name);

			sprintf (str, "%d%s", cmdr->current_cargo[i],
							  unit_name[stock_market[i].units]);

			gba_print (14, y, str);
			y++;
		}		
	}
}

/*********** EQUIP SHIP *********************/

unsigned char equip_buy_laser;

#define FRONT 1
#define REAR 2
#define LEFT 3
#define RIGHT 4

enum equip_types
{
	EQ_FUEL, EQ_MISSILE, EQ_CARGO_BAY, EQ_ECM, EQ_FUEL_SCOOPS,
	EQ_ESCAPE_POD, EQ_ENERGY_BOMB, EQ_ENERGY_UNIT, EQ_DOCK_COMP,
	EQ_GAL_DRIVE, EQ_PULSE_LASER, EQ_BEAM_LASER, EQ_MINING_LASER,
	EQ_MILITARY_LASER, EQ_ID_UNIT
};
	
#define NO_OF_EQUIP_ITEMS	15

struct equip_item
{
	unsigned char canbuy;
	unsigned char y;
	unsigned char show;
	unsigned char level;
	unsigned short int price;
	char *name;
	unsigned int type;
};

struct equip_item equip_stock[NO_OF_EQUIP_ITEMS] =
{
	{0, 0, 1, 1,     2, " Fuel",					EQ_FUEL},
	{0, 0, 1, 1,   300, " Missile",					EQ_MISSILE},
	{0, 0, 1, 1,  4000, " Large Cargo Bay",			EQ_CARGO_BAY},
	{0, 0, 1, 2,  6000, " E.C.M. System",			EQ_ECM},
	{0, 0, 1, 5,  5250, " Fuel Scoops",				EQ_FUEL_SCOOPS},
	{0, 0, 1, 6, 10000, " Escape Pod",				EQ_ESCAPE_POD},
	{0, 0, 1, 7,  9000, " Energy Bomb",				EQ_ENERGY_BOMB},
	{0, 0, 1, 8, 15000, " Extra Energy Unit",		EQ_ENERGY_UNIT},
	{0, 0, 1, 9, 15000, " Docking Computer",		EQ_DOCK_COMP},
	{0, 0, 1, 6,  4000, " ID Computer",	            EQ_ID_UNIT},
	{0, 0, 1,10, 50000, " Galactic Hyperdrive",		EQ_GAL_DRIVE},
	{0, 0, 0, 3,  4000, "+Pulse Laser",				EQ_PULSE_LASER},
	{0, 0, 1, 4, 10000, "+Beam Laser",				EQ_BEAM_LASER},
	{0, 0, 1,10,  8000, "+Mining Laser",			EQ_MINING_LASER},
	{0, 0, 1,10, 60000, "+Military Laser",			EQ_MILITARY_LASER},
};


unsigned char equip_present (int type)
{
	switch (type)
	{
		case EQ_FUEL:
			return (cmdr->fuel >= 70);
		
		case EQ_MISSILE:
			return (cmdr->missiles >= 4);
		
		case EQ_CARGO_BAY:
			return (cmdr->cargo_capacity > 20);
		
		case EQ_ECM:
			return ((cmdr->equipment & ECM) == ECM);
		
		case EQ_FUEL_SCOOPS:
			return ((cmdr->equipment & FUEL_SCOOP) == FUEL_SCOOP);
		
		case EQ_ESCAPE_POD:
			return ((cmdr->equipment & ESCAPE_POD) == ESCAPE_POD);
		
		case EQ_ENERGY_BOMB:
			return ((cmdr->equipment & ENERGY_BOMB) == ENERGY_BOMB);

		case EQ_ENERGY_UNIT:
			return ((cmdr->equipment & ENERGY_UNIT) == ENERGY_UNIT);

		case EQ_DOCK_COMP:
			return ((cmdr->equipment & DOCKING_COMPUTER) == DOCKING_COMPUTER);
			
		case EQ_ID_UNIT:
			return ((cmdr->equipment & ID_UNIT) == ID_UNIT);

		case EQ_GAL_DRIVE:
			return ((cmdr->equipment & GALACTIC_HYPERDRIVE ) == GALACTIC_HYPERDRIVE );
		
		case EQ_PULSE_LASER:
		  // return if we have a space for pulse lasers
		  if (cmdr->front_laser == PULSE_LASER && cmdr->rear_laser == PULSE_LASER &&
		      cmdr->left_laser == PULSE_LASER && cmdr->right_laser == PULSE_LASER)
		    return 1;
		  else
		    return 0;  
		case EQ_BEAM_LASER:
		  // return if we have a space for beam lasers
		  if (cmdr->front_laser == BEAM_LASER && cmdr->rear_laser == BEAM_LASER &&
		      cmdr->left_laser == BEAM_LASER && cmdr->right_laser == BEAM_LASER)
		    return 1;
		  else
		    return 0;
		case EQ_MINING_LASER:
		  // return if we have a space for min lasers
		  if (cmdr->front_laser == MINING_LASER && cmdr->rear_laser == MINING_LASER &&
		      cmdr->left_laser == MINING_LASER && cmdr->right_laser == MINING_LASER)
		    return 1;
		  else
		    return 0;
		case EQ_MILITARY_LASER:
		  // return if we have a space for mil lasers
		  if (cmdr->front_laser == MILITARY_LASER && cmdr->rear_laser == MILITARY_LASER &&
		      cmdr->left_laser == MILITARY_LASER && cmdr->right_laser == MILITARY_LASER)
		    return 1;
		  else
		    return 0;
	}

	return 0;
}

void display_equip_price (int i)
{
	int x, y;
	int col;
	char str[100];
	
	y = equip_stock[i].y;
	if (y == 0)
		return;
  
  if (hilite_item == i)  {
    col = equip_stock[i].canbuy ? GFX_COL_LIGHT_GREEN : GFX_COL_DARK_RED;
  } else {
	  col = equip_stock[i].canbuy ? GFX_COL_WHITE : GFX_COL_GREY;
  }
	x = 2; 
  
  gba_setcolour(col);
  gba_print(x,y,&equip_stock[i].name[1]);

  if (equip_stock[i].price != 0)
  {
    sprintf (str, "%4.0d.%d", (int)gba_div(equip_stock[i].price , 10), (int)(equip_stock[i].price % 10));
    gba_print(22,y,str);
  }
}


void collapse_equip_list (void)
{
	int i;
	int ch;
	
	for (i = 0; i < NO_OF_EQUIP_ITEMS; i++)
	{
		ch = *(equip_stock[i].name);
		equip_stock[i].show = ((ch == ' ') || (ch == '+'));
	}
}

void list_equip_prices (void)
{
	unsigned char i;
	unsigned char y;
	unsigned char tech_level;
	char str[30];

	tech_level = current_planet_data.techlevel + 1;

	equip_stock[0].price = (70 - cmdr->fuel) << 1;
	
	y = 2;
	for (i = 0; i < NO_OF_EQUIP_ITEMS; i++)
	{
	    equip_stock[i].canbuy = ((equip_present (equip_stock[i].type) == 0) &&
								 (equip_stock[i].price <= cmdr->credits));
	
		if (equip_stock[i].show && (tech_level >= equip_stock[i].level))
		{
			equip_stock[i].y = y;
			y ++;
		}
		else
			equip_stock[i].y = 0;

		display_equip_price (i);
	}
	sprintf (str, "Cash: %d.%d Cr", (int)gba_div(cmdr->credits , 10), (int)(cmdr->credits % 10));
	gba_setcolour(GFX_COL_WHITE);
	gba_print(4,19,str);
}


void equip_ship (void)
{
	current_screen = SCR_EQUIP_SHIP;
	
	if (game_draw_flag & RADAR_DRAWN) {
      remove_all_scanner_gfx();
    }
  if (game_draw_flag & MENU_DRAWN) {
    equip_buy_laser = 0;
    hilite_item = 0;
  }
  
  gba_clsUnroll();
  gba_setcolour(GFX_COL_GOLD);
  gba_print(10,0,"EQUIP SHIP");
	gba_scanline (2, 160-9, 236);

	collapse_equip_list();
	list_equip_prices();
	
	if (equip_buy_laser != 0) {
	  char str[30];
	  sprintf (str, "Where? %s%s%s%s",
	  check_laser_purchase(FRONT)?"FRONT ":"", 
	  check_laser_purchase(REAR)?"REAR ":"", 
	  check_laser_purchase(LEFT)?"LEFT ":"", 
	  check_laser_purchase(RIGHT)?"RIGHT ":""
	  );
	  gba_setcolour(GFX_COL_WHITE);
	  gba_print(1,17,str);
	  
	  // this is a  little reminder of the key to press
	  sprintf (str, "Press: %s%s%s%s",
	  check_laser_purchase(FRONT)?"UP    ":"", 
	  check_laser_purchase(REAR)?"DOWN ":"", 
	  check_laser_purchase(LEFT)?"LEFT ":"", 
	  check_laser_purchase(RIGHT)?"RIGHT ":""
	  );
	  gba_setcolour(GFX_COL_LIGHT_GREEN);
	  gba_print(1,18,str);
	}
}


void equip_up(void) {
  if (equip_buy_laser == 0) {
    select_previous_equip();  
  }
  else {
    // buying a laser, the type is given by the value of equip_buy_laser
    if (check_laser_purchase(FRONT)){
      equip_buy_laser = FRONT;
      buy_equip();
    }
  }
}

void equip_down(void) {
  if (equip_buy_laser == 0) {
    select_next_equip();  
  }
  else {
    // buying a laser, the type is given by the value..
    if (check_laser_purchase(REAR)){
      equip_buy_laser = REAR;
      buy_equip();
    }
  }
}

void equip_left(void) {
  if (equip_buy_laser != 0) {
    // buying a laser, the type is given by the value..
    if (check_laser_purchase(LEFT)){
      equip_buy_laser = LEFT;
      buy_equip();
    }
  }
}

void equip_right(void) {
  if (equip_buy_laser != 0) {
    // buying a laser, the type is given by the value..
    if (check_laser_purchase(RIGHT)){
      equip_buy_laser = RIGHT;
      buy_equip();
    }
  }
}


unsigned char check_laser_purchase(unsigned char dir) {
  
  switch (equip_stock[hilite_item].type) {
    case EQ_PULSE_LASER:
		  // return if we have a space for pulse lasers
		  switch (dir) {
		    case FRONT: 
		      if (cmdr->front_laser != PULSE_LASER)
		        return 1;
		      else
		        return 0;
		    case REAR: 
		      if (cmdr->rear_laser != PULSE_LASER)
		        return 1;
          else return 0; 
		    case LEFT: 
		      if (cmdr->left_laser != PULSE_LASER)
		        return 1;
          else return 0;
		    case RIGHT: 
		      if (cmdr->right_laser != PULSE_LASER)
		        return 1;
          else return 0;
		  }
		  break;
		case EQ_BEAM_LASER:
		  // return if we have a space for beam lasers
		  switch (dir) {
		    case FRONT: 
		      if (cmdr->front_laser != BEAM_LASER)
		        return 1;
          else return 0;
		    case REAR: 
		      if (cmdr->rear_laser != BEAM_LASER)
		        return 1;
          else return 0;
		    case LEFT: 
		      if (cmdr->left_laser != BEAM_LASER)
		        return 1;
          else return 0;
		    case RIGHT: 
		      if (cmdr->right_laser != BEAM_LASER)
		        return 1;
          else return 0;
		  }
		  break;
		case EQ_MINING_LASER:
		  // return if we have a space for mining lasers
		  switch (dir) {
		    case FRONT: 
		      if (cmdr->front_laser != MINING_LASER)
		        return 1;
          else return 0;
		    case REAR: 
		      if (cmdr->rear_laser != MINING_LASER)
		        return 1;
          else return 0;
		    case LEFT: 
		      if (cmdr->left_laser != MINING_LASER)
		        return 1;
          else return 0;
		    case RIGHT: 
		      if (cmdr->right_laser != MINING_LASER)
		        return 1;
          else return 0;
		  }
		  break;
		case EQ_MILITARY_LASER:
		  // return if we have a space for mil lasers
		  switch (dir) {
		    case FRONT: 
		      if (cmdr->front_laser != MILITARY_LASER)
		        return 1;
          else return 0;
		    case REAR: 
		      if (cmdr->rear_laser != MILITARY_LASER)
		        return 1;
          else return 0;
		    case LEFT: 
		      if (cmdr->left_laser != MILITARY_LASER)
		        return 1;
          else return 0;
		    case RIGHT: 
		      if (cmdr->right_laser != MILITARY_LASER)
		        return 1;
          else return 0;
		  }
		  break;
  }

  return 0;
}


void select_next_equip (void)
{
	int next;
	int i;

	if (hilite_item == (NO_OF_EQUIP_ITEMS - 1))
		return;

	next = hilite_item;
	for (i = hilite_item + 1; i < NO_OF_EQUIP_ITEMS; i++)
	{
		if (equip_stock[i].y != 0)
		{
			next = i;
			break;
		}
	}
  hilite_item = next;
}

void select_previous_equip (void)
{
	int i;
	int prev;
	
	if (hilite_item == 0)
		return;
	
	prev = hilite_item;
	for (i = hilite_item - 1; i >= 0; i--)
	{
		if (equip_stock[i].y != 0)
		{
			prev = i;
			break;
		}
	}
  hilite_item = prev;
}


int laser_refund (int laser_type)
{
	switch (laser_type)
	{
		case PULSE_LASER:
			return 4000;
		
		case BEAM_LASER:
			return 10000;
		
		case MILITARY_LASER:
			return 60000;
		
		case MINING_LASER:
			return 8000;
	}

	return 0;
}


void buy_equip (void)
{
	if (equip_stock[hilite_item].canbuy == 0)
		return;

	if (equip_stock[hilite_item].name[0] == '+' && equip_buy_laser == 0)
	{
	    // a laser
        equip_buy_laser = hilite_item;
        
		return;		
	}

	switch (equip_stock[hilite_item].type)
	{
		case EQ_FUEL:
			cmdr->fuel = myship.max_fuel;
			update_console();
			break;

		case EQ_MISSILE:
			cmdr->missiles++;
			update_console();
			break;
		
		case EQ_CARGO_BAY:
			cmdr->cargo_capacity = 35;
			break;
		
		case EQ_ECM:
		  if (!(cmdr->equipment & ECM))
		    cmdr->equipment |= ECM;
			break;
		
		case EQ_FUEL_SCOOPS:
		  if (!(cmdr->equipment & FUEL_SCOOP))
		    cmdr->equipment |= FUEL_SCOOP;
			break;
		
		case EQ_ESCAPE_POD:
			if (!(cmdr->equipment & ESCAPE_POD))
		    cmdr->equipment |= ESCAPE_POD;
			break;
		
		case EQ_ENERGY_BOMB:
			if (!(cmdr->equipment & ENERGY_BOMB))
		    cmdr->equipment |= ENERGY_BOMB;
			break;

		case EQ_ENERGY_UNIT:
			if (!(cmdr->equipment & ENERGY_UNIT))
		    cmdr->equipment |= ENERGY_UNIT;
			break;
			
		case EQ_DOCK_COMP:
			if (!(cmdr->equipment & DOCKING_COMPUTER))
		    cmdr->equipment |= DOCKING_COMPUTER;
			break;
			
		case EQ_ID_UNIT:
			if (!(cmdr->equipment & ID_UNIT))
		    cmdr->equipment |= ID_UNIT;
			break;

		case EQ_GAL_DRIVE:
			if (!(cmdr->equipment & GALACTIC_HYPERDRIVE))
		    cmdr->equipment |= GALACTIC_HYPERDRIVE;
			break;
			
		case EQ_PULSE_LASER:
		  // return if we have a space for pulse lasers
		  switch (equip_buy_laser) {
		    case FRONT: 
		      cmdr->credits += laser_refund (cmdr->front_laser);
			    cmdr->front_laser = PULSE_LASER;
		      break;
		    case REAR: 
		      cmdr->credits += laser_refund (cmdr->rear_laser);
			    cmdr->rear_laser = PULSE_LASER;
		      break;
		    case LEFT: 
		      cmdr->credits += laser_refund (cmdr->left_laser);
			    cmdr->left_laser = PULSE_LASER;
		      break;
		    case RIGHT: 
		      cmdr->credits += laser_refund (cmdr->right_laser);
			    cmdr->right_laser = PULSE_LASER;
		      break;
		  }
		  break;
		case EQ_BEAM_LASER:
		  switch (equip_buy_laser) {
		    case FRONT: 
		      cmdr->credits += laser_refund (cmdr->front_laser);
			    cmdr->front_laser = BEAM_LASER;
		      break;
		    case REAR: 
		      cmdr->credits += laser_refund (cmdr->rear_laser);
			    cmdr->rear_laser = BEAM_LASER;
		      break;
		    case LEFT: 
		      cmdr->credits += laser_refund (cmdr->left_laser);
			    cmdr->left_laser = BEAM_LASER;
		      break;
		    case RIGHT: 
		      cmdr->credits += laser_refund (cmdr->right_laser);
			    cmdr->right_laser = BEAM_LASER;
		      break;
		  }
		  break;
		case EQ_MINING_LASER:
		  switch (equip_buy_laser) {
		    case FRONT: 
		      cmdr->credits += laser_refund (cmdr->front_laser);
			    cmdr->front_laser = MINING_LASER;
		      break;
		    case REAR: 
		      cmdr->credits += laser_refund (cmdr->rear_laser);
			    cmdr->rear_laser = MINING_LASER;
		      break;
		    case LEFT: 
		      cmdr->credits += laser_refund (cmdr->left_laser);
			    cmdr->left_laser = MINING_LASER;
		      break;
		    case RIGHT: 
		      cmdr->credits += laser_refund (cmdr->right_laser);
			    cmdr->right_laser = MINING_LASER;
		      break;
		  }
		  break;
		case EQ_MILITARY_LASER:
		  switch (equip_buy_laser) {
		    case FRONT: 
		      cmdr->credits += laser_refund (cmdr->front_laser);
			    cmdr->front_laser = MILITARY_LASER;
		      break;
		    case REAR: 
		      cmdr->credits += laser_refund (cmdr->rear_laser);
			    cmdr->rear_laser = MILITARY_LASER;
		      break;
		    case LEFT: 
		      cmdr->credits += laser_refund (cmdr->left_laser);
			    cmdr->left_laser = MILITARY_LASER;
		      break;
		    case RIGHT: 
		      cmdr->credits += laser_refund (cmdr->right_laser);
			    cmdr->right_laser = MILITARY_LASER;
		      break;
		  }
		  break;
	}
  equip_buy_laser = 0;
	cmdr->credits -= equip_stock[hilite_item].price;
	PlaySoundFX(SND_SELECT);
	list_equip_prices();
}


/************ GALAXY **************************/


// the galaxy is 128 wide by 64 high
// won't fit on screen at double size and can't 
// double its height because the fuel circle would have to be oval
void display_galactic_chart (void)
{
  unsigned short int i;
  struct galaxy_seed glx;
  char str[64];
  unsigned char px,py;

  short_range_flag = 0;

  current_screen = SCR_GALACTIC_CHART;
  if (game_draw_flag & RADAR_DRAWN) {
    remove_all_scanner_gfx();
  }

  gba_clsUnroll();

  sprintf (str, "GALACTIC CHART %d", cmdr->galaxy_number + 1);

  gba_setcolour(GFX_COL_GOLD);
  gba_print(7,0,str);
  gba_scanline (2, 160-9, 236);

  if (game_state_flag & FIND_PLANET) {
    // show the find planet thing
    unsigned char len; 
    gba_setcolour(GFX_COL_WHITE);
    sprintf(str, "Planet Name? %s",find_name);
    len = strlen (str);
    gba_print(2,18,str);
    gba_setcolour(GFX_COL_LIGHT_GREEN);
    gba_print(2+len,18,find_char);

  }

  draw_fuel_limit_circle ( (times_GALAXY_RESIZE_FACTOR(docked_planet.d) >> 1)+GALAXY_X_OFFSET,
      (times_GALAXY_RESIZE_FACTOR(docked_planet.b) >> 2) +GALAXY_Y_OFFSET);

  glx = cmdr->galaxy;

  gba_setcolour(GFX_COL_WHITE);
  gba_scanline (2, 161-(GALAXY_Y_OFFSET), 236);
  gba_scanline (2, 158-(GALAXY_Y_OFFSET+times_GALAXY_RESIZE_FACTOR(64)), 236);
  for (i = 0; i < 256; i++)
  {
    px = (times_GALAXY_RESIZE_FACTOR(glx.d) >> 1)+GALAXY_X_OFFSET;
    py = (times_GALAXY_RESIZE_FACTOR(glx.b) >> 2)+GALAXY_Y_OFFSET;

    gba_drawpixel(px,py);

    waggle_galaxy (&glx);
    waggle_galaxy (&glx);
    waggle_galaxy (&glx);
    waggle_galaxy (&glx);

  }

  if (game_draw_flag & MENU_DRAWN) {  
    cross_x = (times_GALAXY_RESIZE_FACTOR(hyperspace_planet.d) >> 1);
    cross_y = (times_GALAXY_RESIZE_FACTOR(hyperspace_planet.b) >> 2);

    strcpy (planet_name_for_display,"                ");
    light_years_for_display = 0;

  }

  gba_setcolour(GFX_COL_WHITE);
  gfx_draw_spot(cross_x+GALAXY_X_OFFSET,cross_y+GALAXY_Y_OFFSET);

  if (!(game_state_flag & FIND_PLANET))
    gba_print(1,18,planet_name_for_display);

  if (light_years_for_display > 0) {
    char str[31];
    sprintf (str, "Distance: %2d.%d Light Years", 
        (int)gba_div(light_years_for_display , 10), light_years_for_display % 10);
    //    gba_setcolour(GFX_COL_WHITE);
    gba_print (1, 19, str);

    // debug to show the planet numbers
    //    unsigned char pnum;
    //    pnum = find_planet_number (hyperspace_planet);
    //    sprintf (str, "Pnums: %d %d", hyperspace_planet.d, hyperspace_planet.b);
    //    gba_print (1, 4, str);

  } else if (light_years_for_display < 0) {
    gba_print (1, 19, "Unknown Planet");
  }


}


void draw_fuel_limit_circle (unsigned char cx, unsigned char cy)
{
	unsigned char radius;
	
	if (current_screen == SCR_GALACTIC_CHART)
	{
		radius = times_GALAXY_RESIZE_FACTOR(cmdr->fuel) / 8;
		gba_setcolour(GFX_COL_GOLD);
    gfx_draw_spot(cx,cy);
	}
	else
	{
		radius = cmdr->fuel;	
    gba_setcolour(GFX_COL_GOLD);
    gba_vertline (cx, cy-16,32);
    gba_horizline (cx-16, cy,32);
	}
	
	gba_setcolour(GFX_COL_LIGHT_GREEN);
	gba_circle(radius, cx, cy, 0);
  
}


void find_planet_by_name (char *find_name)
{
  unsigned short i;
	struct galaxy_seed glx;
	char planet_name[16];
	signed char found;
	
	glx = cmdr->galaxy;
	found = 0;
	
	for (i = 0; i < 256; i++)
	{
		name_planet (planet_name, glx);
		
		if (strcmp (planet_name, find_name) == 0)
		{
			found = 1;
			break;
		}

		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
		waggle_galaxy (&glx);
	}

	if (!found)
	{
	  // show unknown planet string
	  light_years_for_display = -1;
		return;
	}


	hyperspace_planet = glx;

//   sprintf (str, "%-18s", planet_name);
//	 show_distance (19, &docked_planet, &hyperspace_planet);

	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = times_GALAXY_RESIZE_FACTOR(hyperspace_planet.d)>>1;
		cross_y = times_GALAXY_RESIZE_FACTOR(hyperspace_planet.b)>>2;
	}
	else
	{
		cross_x = (hyperspace_planet.d - docked_planet.d) << 2;
		cross_y = (hyperspace_planet.b - docked_planet.b) << 1;
	}

    show_distance_to_planet();  

}

unsigned short calc_distance_to_planet (struct galaxy_seed *from_planet, struct galaxy_seed *to_planet)
{
	unsigned long int dx,dy;
	unsigned short int light_years;

	dx = abs(to_planet->d - from_planet->d) << 2;
	dy = abs(to_planet->b - from_planet->b) << 1;

	dx = dx * dx;
	dy = dy * dy;

	light_years = gba_sqrt(dx + dy);
	
	return light_years;
}


void show_distance (unsigned char ypos, struct galaxy_seed *from_planet, struct galaxy_seed *to_planet)
{
	unsigned short light_years;
	
	light_years = calc_distance_to_planet (from_planet, to_planet);
	
	light_years_for_display = light_years;

}

void move_cursor_to_origin (void)
{
  if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = times_GALAXY_RESIZE_FACTOR(docked_planet.d)>>1;
		cross_y = times_GALAXY_RESIZE_FACTOR(docked_planet.b)>>2;
	}
	else
	{
		cross_x = 0;
		cross_y = 0;
	}
  PlaySoundFX(SND_CANCEL);
    
	show_distance_to_planet();
}


void show_distance_to_planet (void)
{
	unsigned char px,py;
	char planet_name[16];
//	char str[32];

	if (current_screen == SCR_GALACTIC_CHART)
	{
		px = times_GALAXY_RESIZE_DIVISOR(cross_x<<1);
		py = times_GALAXY_RESIZE_DIVISOR(cross_y<<2);
	}
	else
	{
		px = (cross_x >> 2) + docked_planet.d;
		py = (cross_y >> 1) + docked_planet.b;
	}

	hyperspace_planet = find_planet (px, py);

	name_planet (planet_name, hyperspace_planet);


	sprintf (planet_name_for_display, "%s", planet_name);

	show_distance (19, &docked_planet, &hyperspace_planet);

	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x = times_GALAXY_RESIZE_FACTOR(hyperspace_planet.d)>>1;
		cross_y = times_GALAXY_RESIZE_FACTOR(hyperspace_planet.b)>>2;		
	}
	else
	{
		cross_x = (hyperspace_planet.d - docked_planet.d) << 2;
		cross_y = (hyperspace_planet.b - docked_planet.b) << 1;
	}
	
}


/************ SHORT RANGE  **************************/
void set_cross_short_range(void) {
  cross_x = ((hyperspace_planet.d - docked_planet.d)<<2);
  cross_y = ((hyperspace_planet.b - docked_planet.b)<<1);
  	
  light_years_for_display = 0;
}



void display_short_range_chart (void)
{
  unsigned short i, j, plot_pass;
  struct galaxy_seed glx;
  static struct galaxy_seed glxs[256];
  static unsigned char update_seed_table;
  signed short px,py;
  char planet_name[16];
  short int row_used_left[64], row_used_right[64]; 
  // first and last column of space occupied in each row
  // should enable several labels in each row
  unsigned short int row, column, last_column, blob_row, blob_column;
  unsigned char blob_size;
  int ldx = 0; int ldy = 0;
  const struct {
    int x;
    int y;
  } label_positions[] = {
    { 0,  0}, {-1, -1}, {-1, +1}, { 0, +1}, 
    { 0, -1}, {+1, -1}, {+1,  0}, {+1, +1}, 
    {-2, -1}, {-2, +1}, {-3, -1}, {-3, +1},
    {-4, -1}, {-4, +1},
    {100, 0}, {100, -1}, {100, +1}, {101, +1}, 
    {101, -1}, { 99, -1}, { 99, 0}, { 99, +1}, 
    {102, -1}, {102, +1}, {103, -1}, {103, +1}, 
    {-1,  0}, {-2,  0}, {-3,  0}, {-4,  0}, 
    {101, 0}, {102, 0}, {103, 0}, {104, 0}, 
    {1000, 1000}
  };  

  // used for calculation of blob size
  int radius; // planet's radius
  const float r6656 = 4, r2816 = 2; // radii in pixels of planets with 6656 and 2816 km radius
  const float a = (r6656 - r2816)/(6656.0-2816.0), b = r2816 - 2816 * a; // linear scaling

  short_range_flag = 1;
  if (current_screen != SCR_SHORT_RANGE) {
    current_screen = SCR_SHORT_RANGE;
    update_seed_table = 1;
  }

  if (game_draw_flag & RADAR_DRAWN) {
    remove_all_scanner_gfx();
  }

  gba_clsUnroll();
  gba_setcolour(GFX_COL_GOLD);
  gba_print(6,0,"SHORT RANGE CHART");
  gba_scanline (2, 160-9, 236);

  for (i = 0; i < 64; i++) {
    row_used_left[i] = 999;
    row_used_right[i] = -1;
  }

  for (plot_pass = 1; plot_pass <= 4; plot_pass++) {
    // plot_pass 1: Build up galaxy seed table and plot blobs
    //           2: Draw fuel limit circle
    //           3: Plot long labels (more than 6 characters)
    //           4: Plot short labels

    if (plot_pass == 2) {
      draw_fuel_limit_circle (GFX_X_CENTRE, GFX_Y_CENTRE);
      update_seed_table = 0;
      continue;
    }

    glx = cmdr->galaxy;

    for (i = 0; i < 256; i++)
    {
      if (plot_pass == 1 && update_seed_table) {
        glxs[i] = glx;
        waggle_galaxy (&glx);
        waggle_galaxy (&glx);
        waggle_galaxy (&glx);
        waggle_galaxy (&glx);
      }
      else
        glx = glxs[i];

      if ((abs (glx.d - docked_planet.d) >= 20) || 
          (glx.b - docked_planet.b >= 36) ||
          (glx.b - docked_planet.b <= -38))
      {
        continue;
      }

      px = (glx.d - docked_planet.d);
      px = px*4 + GFX_X_CENTRE;  /* Convert to screen co-ords */

      py = (glx.b - docked_planet.b);
      py = (py<<1) + GFX_Y_CENTRE;	/* Convert to screen co-ords */

      // Calculate blob size
      // blob size corresponds to radius of planet
      radius = ((glx.f & 15) + 11) * 256 + glx.d; // see generate_planet_data()
      blob_size = (int) (a * radius + b + 0.5);

      // Determine planet name
      name_planet (planet_name, glx);
      capitalise_name (planet_name);

      row = py >> 3;
      column = (px >> 3)+1;
      blob_row = row;
      blob_column = px >> 3;
      last_column = column + strlen(planet_name); // name + one space (as separator from adjacent labels)


      if ((plot_pass == 3 && strlen(planet_name) > 6) 
          || (plot_pass == 4 && strlen(planet_name) <= 6)) { // handle labels

        // Find optimal position for label
        for (j=0; label_positions[j].x != 1000; j++) {
          ldx = label_positions[j].x;
          ldy = label_positions[j].y;
          if (ldx > 90) { 
            ldx = ldx - 100 - strlen(planet_name) - 1; // right justified
          }

          if ((last_column+ldx >= row_used_left[row+ldy]
                && column+ldx <= row_used_right[row+ldy])
              // label and occupied region of row overlap?
              || column+ldx < 0 || last_column+ldx > 29
              || row+ldy <= 1 || row+ldy >= 19)
            // label outside range
            continue;
          else
            break;
        }
        if (label_positions[j].x == 1000) 
          row = 0;
        else {
          column += ldx;
          last_column += ldx;
          row += ldy; 
        }

        // Plot label
        if (row > 1 && row < 19)
        {
          // register space occupied by label
          if (column < row_used_left[row])
            row_used_left[row] = column; 
          if (last_column > row_used_right[row])
            row_used_right[row] = last_column;

          // (Determination of planet name moved upwards since needed earlier)

          // plot label
          gba_setcolour(GFX_COL_WHITE);
          gba_print_masked(column,row,planet_name);

        }
      } // /handle labels    

      //      Calculation of blob size moved upwards since needed earlier

      // Plot blob
      if (plot_pass == 1) {
        gba_setcolour(GFX_COL_ORANGE);
        gba_circle(blob_size, px, py, 1); 

        // register space occupied by blob
        if (blob_column < row_used_left[blob_row])
          row_used_left[blob_row] = blob_column; 
        if (blob_column > row_used_right[blob_row])
          row_used_right[blob_row] = blob_column;

      }

    }
  }

  if (game_draw_flag & MENU_DRAWN) {  
    //    cross_x = ((hyperspace_planet.d - docked_planet.d) * 4);
    //  	cross_y = ((hyperspace_planet.b - docked_planet.b)*2);
    //  	
    //  	light_years_for_display = 0;
    set_cross_short_range();
  }

  gba_setcolour(GFX_COL_WHITE);

  if (cross_x > -GFX_Y_CENTRE && cross_x < 240-GFX_X_CENTRE && cross_y > -GFX_Y_CENTRE && cross_y < 160 - GFX_Y_CENTRE) {
    gba_vertline (cross_x+GFX_X_CENTRE, cross_y+GFX_Y_CENTRE-8,16);
    gba_horizline (cross_x+GFX_X_CENTRE-8, cross_y+GFX_Y_CENTRE,16);
  }

  if (light_years_for_display > 0) {
    char str[40];
    sprintf (str, "%s: %2d.%d Light Years", planet_name_for_display,
        (int)gba_div(light_years_for_display , 10), light_years_for_display % 10);
    gba_print (1, 19, str);
  } else {
    gba_print(1,19,planet_name_for_display);
  }


}


/*
 * Displays data on the currently selected Hyperspace Planet.
 */

const static char *economy_type[] = {"Rich Industrial",
						"Average Industrial",
						"Poor Industrial",
						"Mainly Industrial",
						"Mainly Agricultural",
						"Rich Agricultural",
						"Average Agricultural",
						"Poor Agricultural"};

const static char *government_type[] = {	"Anarchy",
							"Feudal",
							"Multi-Government",
							"Dictatorship",
							"Communist",
							"Confederacy",
							"Democracy",
							"Corporate State"};



void display_data_on_planet (void)
{
  char planet_name[16];
	char str[100];
	const char *description;
	struct planet_data hyper_planet_data;
  
    unsigned char y = 0;
  
	current_screen = SCR_PLANET_DATA;
    if (game_draw_flag & RADAR_DRAWN) {
      remove_all_scanner_gfx();
    }
    
	gba_clsUnroll();
	name_planet (planet_name, hyperspace_planet);
	sprintf (str, "DATA ON %s", planet_name);

  gba_setcolour(GFX_COL_GOLD);
  gfx_centre_text(y, str);
  gba_scanline (2, 160-9, 236);
  
  y = 2;
	generate_planet_data (&hyper_planet_data, hyperspace_planet);

	show_distance (42, &docked_planet, &hyperspace_planet);
	
	sprintf (str, "Distance: %d.%d Light Years",
        (int)gba_div(light_years_for_display , 10), (int)(light_years_for_display % 10));
  gba_setcolour(GFX_COL_WHITE);
  gba_print (1, y, str);
	y += 2;
  
	sprintf (str, "Economy: %s", economy_type[hyper_planet_data.economy]);
	gba_print (1, y, str);
	y += 1;

	sprintf (str, "Government: %s", government_type[hyper_planet_data.government]);
	gba_print (1, y, str);
	y += 1;

	sprintf (str, "Tech.Level: %d", hyper_planet_data.techlevel + 1);
	gba_print (1, y, str);
	y += 2;

	sprintf (str, "Population: %d.%d Billion", (int)gba_div(hyper_planet_data.population , 10), 
            (int)(hyper_planet_data.population % 10));
	gba_print (1, y, str);
	y += 1;

	describe_inhabitants (str, hyperspace_planet);
    print_multi_text(1, y, str);
//	gba_print (1, y, str);
	y += (1 + (strlen(str)>29? 1: 0));

	sprintf (str, "Gross Product.: %ld MCr", hyper_planet_data.productivity);
	gba_print (1, y, str);
	y += 1;

	sprintf (str, "Average Radius: %d km", hyper_planet_data.radius);
	gba_print (1, y, str);
	y += 2;

	description = describe_planet (hyperspace_planet);
//	gba_print (1, y, description);
	print_multi_text(1,y, description);
//	gfx_display_pretty_text (16, 298, 400, 384, description);
}

// called when the find planet icon is selected
// it goes to the glactic chart, and sets the relevent flags
void display_find_planet (void) {
  game_state_flag |= FIND_PLANET;
  hilite_find_letter = 0;
  find_char[0] = '?';
  find_char[1] = '\0';
  find_name[0] = '\0';
  display_galactic_chart();
}

void delete_find_char(void) {
  unsigned char len;

	len = strlen (find_name);
	if (len == 0)
		return;
	find_char[0] = 	find_name[len - 1];
	find_name[len - 1] = '\0';
  PlaySoundFX(SND_CANCEL);
}

void add_find_char(void) {
  char str[30];
	
	PlaySoundFX(SND_SELECT);
	if (strlen (find_name) == 16) {
	  finish_find();
	  return;
	}
	if (find_char[0] != '?') {
  	str[0] = toupper (find_char[0]);
  	str[1] = '\0';
  	strcat (find_name, str);
    find_char[0] = '?';
    find_char[1] = '\0';
  } else {
    finish_find();
  }


}

void up_find_char(void) {
  if (find_char[0] == 'Z')
    find_char[0] = '?';
  else if (find_char[0] == '?')
    find_char[0] = 'A'; 
  else
    find_char[0]++;
}

void down_find_char(void) {
  if (find_char[0] == 'A')
    find_char[0] = '?';
  else if (find_char[0] == '?')
    find_char[0] = 'Z'; 
  else
    find_char[0]--; 
}

void finish_find(void) {
  find_planet_by_name (find_name);
  game_state_flag &= ~FIND_PLANET;
}

void galactic_a_pressed(void) {
  // set the docked planet to the nearest...
  show_distance_to_planet();
  docked_planet = hyperspace_planet;
  
}
