#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "gba.h"
#include "gbalib_asm.h"
#include "options.h"
#include "elite.h"
#include "file.h"
#include "saving.h"
#include "trade.h"
#include "sound.h"
#include "main.h"
#include "gfx.h"
#include "docked.h"
#include "missions.h"
#include "swat.h"
#include "shipdata.h"

#include "registers.h"
// added before compilation succeeded, TM

#define MAX_SAVES 8
#define SAVE_LENGTH 81
#define SAVE_SIZE 120
#define SAVE_OFFSET 2
#define SAVE_BACKSET 7 // the amount back from the end of the file that the minutes are..
                       // this is to let me add more stuff in, like options that aren't checksumd
#define QUICK_SAVE_SIZE (SAVE_SIZE + 39 + MAX_UNIV_OBJECTS * sizeof(Object) + 2*(ELITE_SHIPS.nShipCount+1))
                       // number of bytes to be saved + 1 for quick_saved flag (first byte)


#define SAVE_LIST_X_OFFSET 4
#define SAVE_LIST_Y_OFFSET 6

int save_commander_file (unsigned char index);
int load_commander_file (unsigned char index);
int checksum (unsigned char *block);
void set_message(char * txt);

unsigned char hilite_item;
unsigned char hilite_letter;
unsigned long int next_screen;

unsigned char quick_saved = 0;

#define DEFAULT_JAMESON 8
#define QUICK_SAVE 9
#define MAX_LOAD_SLOT 9

// current name to save
char name[8];
// already saved data
unsigned char save_count;
char saved_name[8][8];
unsigned char saved_mins[8];
unsigned short saved_hours[8];
char msg[8];


int checksum (unsigned char *block)
{
	int acc,carry;
	int i;

	acc = SAVE_LENGTH;
	carry = 0;
	for (i = SAVE_LENGTH; i > 0; i--)
	{
		acc += block[i-1] + carry;
		carry = acc >> 8;
		acc &= 255;
		acc ^= block[i];
	}

	return acc;
}

int save_commander_file (unsigned char index)
{
  // Disable interrupts
  REG_IME = 0x00;                       
 
  unsigned char * block;
  unsigned short i;
  unsigned char chk;

  unsigned short bi, bii, sbi; // block index
  unsigned short total_size = SAVE_SIZE*MAX_SAVES + 10 + QUICK_SAVE_SIZE;

  block = (unsigned char *) malloc (sizeof(char) * total_size);
  // start index...
  if (index == QUICK_SAVE)
      index = 8;
  bi = SAVE_OFFSET+ index * SAVE_SIZE;
  sbi = bi;

  LoadData(block, "ELT");
  
  if (block[0] == 0) {
    // failed to load from sram
    for (i = 0; i < total_size; i++)
      block[i] = 0; 
  }
  
  // each bit says if the save is used or not
  // 8 bits = 8 saves...
  block[1] = save_count | (1<<index);
  
  for (i = 0; i < 7; i++) {
    block[bi++] = name[i];
    cmdr->name[i] = name[i];
  }
    
// bi + 7    
    
	block[bi++] = cmdr->mission;
	block[bi++] = cmdr->ship_x;
	block[bi++] = cmdr->ship_y;
	block[bi++] = cmdr->galaxy.a;
	block[bi++] = cmdr->galaxy.b;
	block[bi++] = cmdr->galaxy.c;
	block[bi++] = cmdr->galaxy.d;
	block[bi++] = cmdr->galaxy.e;
	block[bi++] = cmdr->galaxy.f;
	block[bi++] = (cmdr->credits >> 24) & 255;
	block[bi++] = (cmdr->credits >> 16) & 255;
	block[bi++] = (cmdr->credits >> 8) & 255;
	block[bi++] = cmdr->credits & 255;
	block[bi++] = cmdr->fuel;
	block[bi++] = 4;  
	block[bi++] = cmdr->galaxy_number;
	block[bi++] = cmdr->front_laser;
	block[bi++] = cmdr->rear_laser;
	block[bi++] = cmdr->left_laser;
	block[bi++] = cmdr->right_laser;

// bi + 7 + 20 = 27
	
	// a hack here... add the gba palette/solid gfx/instant dock flags
	u8 options_flag = 0;
	if (game_state_flag & INSTANT_DOCK) 
	  options_flag |= 1;
	if (game_state_flag & SOLID_SHIPS) 
	  options_flag |= 2;
	if (game_state_flag & GBA_PALETTE) 
	  options_flag |= 4;
	if (game_state_flag & SHOW_PROGRESS) 
	  options_flag |= 8;
	if (game_state_flag & EASY_MODE) 
	  options_flag |= 16;
	if (game_state_flag & DISTANT_DOT)
	  options_flag |= 32;
	if (game_state_flag & LUCKY_DOCK_COMP)
	  options_flag |= 64;
	if (game_state_flag & DOCK_MUSIC)
	  options_flag |= 128;
	
	
	block[bi++] = options_flag;
	block[bi++] = 0;
	block[bi++] = cmdr->cargo_capacity + 2;
	// bi + 7 + 20 + 3 = 30

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		block[bi++] = cmdr->current_cargo[i];
    
    // bi + 7 + 20 + 3 + 17 = 47
    	
	block[bi++] = (cmdr->equipment >> 24) & 255;
	block[bi++] = (cmdr->equipment >> 16) & 255;
	block[bi++] = (cmdr->equipment >> 8) & 255;
	block[bi++] = cmdr->equipment & 255;
	
	block[bi++] = cmdr->missiles;
	block[bi++] = cmdr->legal_status;
	// bi + 7 + 20 + 3 + 17 + 6 = 53
	
//	sprintf(str, "%d %d",bi, stock_market[0].current_quantity);
//	set_message(str);
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++) {
		block[bi++] = stock_market[i].current_quantity;
		saved_cmdr->station_stock[i] = stock_market[i].current_quantity;
	}
	
	// bi + 7 + 20 + 3 + 17 + 6 + 17 = 70
	
	block[bi++] = cmdr->market_rnd;
	block[bi++] = cmdr->score & 255;
	block[bi++] = cmdr->score >> 8;
	block[bi++] = 0x20;
	
	// bi + 7 + 20 + 3 + 17 + 6+ 17 + 4 = 74

	block[bi++] = cmdr->mission3_planet_number;
	block[bi++] = cmdr->mission3_galaxy_number;
	
	// bi + 7 + 20 + 3 + 17 + 6+ 17 + 4 + 2 = 76
        
    get_current_controls(&block[bi],&block[bi+1],&block[bi+2],&block[bi+3]);
    bi+=4;

	// bi + 7 + 20 + 3 + 17 + 6+ 17 + 4 + 2 + 4 = 80

	block[bi++] = cmdr->mission_counter;

	// bi + 7 + 20 + 3 + 17 + 6+ 17 + 4 + 2 + 4 + 1 = 81
    	
	chk = checksum (&block[sbi]);
	block[sbi+SAVE_LENGTH+1] = chk ^ 0xA9;
	block[sbi+SAVE_LENGTH+2] = chk;
    
    bii = SAVE_OFFSET+(index+1)*SAVE_SIZE - SAVE_BACKSET - 1;
    block[bii++] = seconds;
    block[bii++] = minutes;
	block[bii++] = hours &255;
    block[bii++] = hours >>8;

    if (index > 7) {
        // quick save data
        bi = total_size - QUICK_SAVE_SIZE + SAVE_SIZE;
        block[bi++] = quick_saved = 1;
        block[bi++] = zoom;
        block[bi++] = laser_temp & 255;
        block[bi++] = laser_temp >> 8;
        block[bi++] = (flight_speed + 32768) & 255;
        block[bi++] = (flight_speed + 32768) >> 8;
        block[bi++] = (flight_roll + 32768) & 255;
        block[bi++] = (flight_roll + 32768) >> 8;
        block[bi++] = (flight_climb + 32768) & 255;
        block[bi++] = (flight_climb + 32768) >> 8;
        block[bi++] = game_state_flag & 255;
        block[bi++] = (game_state_flag >> 8) & 255;
        block[bi++] = (game_state_flag >> 16) & 255;
        block[bi++] = (game_state_flag >> 24) & 255;
        block[bi++] = game_draw_flag & 255;
        block[bi++] = (game_draw_flag >> 8) & 255;
        block[bi++] = (game_draw_flag >> 16) & 255;
        block[bi++] = (game_draw_flag >> 24) & 255;
        block[bi++] = front_shield & 255;
        block[bi++] = front_shield >> 8;
        block[bi++] = aft_shield & 255;
        block[bi++] = aft_shield >> 8;
        block[bi++] = (energy + 32768) & 255;
        block[bi++] = (energy + 32768) >> 8;
        block[bi++] = swat_flag;    
        block[bi++] = missile_target + 128;
        block[bi++] = ecm_active + 128;
        block[bi++] = docked_planet.a;
        block[bi++] = docked_planet.b;
        block[bi++] = docked_planet.c;
        block[bi++] = docked_planet.d;
        block[bi++] = docked_planet.e;
        block[bi++] = docked_planet.f;
        block[bi++] = hyperspace_planet.a;
        block[bi++] = hyperspace_planet.b;
        block[bi++] = hyperspace_planet.c;
        block[bi++] = hyperspace_planet.d;
        block[bi++] = hyperspace_planet.e;
        block[bi++] = hyperspace_planet.f;
        
        for (i = 0; i < MAX_UNIV_OBJECTS * sizeof(Object); i++)
            block[bi++] = ((unsigned char*)universe)[i];
        for (i = 0; i <= ELITE_SHIPS.nShipCount; i++) {
            block[bi++] = ship_count[i] & 255;
            block[bi++] = ship_count[i] >> 8;
        }

    }    
    
  SaveData(&block[1], total_size-4, "ELT");
  free(block);
  // Enable interrupts
  REG_IME = 0x1;                    
  return 1;
	
}


int load_commander_file (unsigned char index)
{
  
//  char str[20];
  // disable interrupts
  REG_IME = 0x0;
  unsigned char * block;
  unsigned short i;
	signed long int chk;
	unsigned char chk_block1;
	unsigned char chk_block2;
  unsigned short bi, bii; // block index
  unsigned short sbi; // start block index
  struct galaxy_seed planet;
  
  unsigned short total_size = SAVE_SIZE*MAX_SAVES + 10 + QUICK_SAVE_SIZE;
  block = (unsigned char *) malloc (sizeof(char) * total_size);
  
  if (index == QUICK_SAVE)
      index = 8;
  bi = SAVE_OFFSET+ index * SAVE_SIZE;
  sbi = bi;
  
  LoadData(block, "ELT");
  if (block[0] == 0) {
    // error loading data from sram
    free(block);
    return 0;
  }
  
  
  chk = checksum (&block[sbi]);
  chk_block1 = block[sbi+SAVE_LENGTH+1];
  chk_block2 = block[sbi+SAVE_LENGTH+2];
  
  
// Check checksum
	if ((chk_block1 != (chk ^ 0xA9)) || (chk_block2 != chk)) {
	  free(block);
	  // Enable interrupts
      REG_IME = 0x1;
	  return 0;
	}
	
	
  for (i = 0; i < 7; i++) {
    cmdr->name[i] = block[bi++];
  }
    cmdr->name[7] = 0;
    
	cmdr->mission  = block[bi++];
	cmdr->ship_x   = block[bi++];
	cmdr->ship_y   = block[bi++];
	cmdr->galaxy.a = block[bi++];
	cmdr->galaxy.b = block[bi++];
	cmdr->galaxy.c = block[bi++];
	cmdr->galaxy.d = block[bi++];
	cmdr->galaxy.e = block[bi++];
	cmdr->galaxy.f = block[bi++];
	cmdr->credits = block[bi++] << 24;
	cmdr->credits += block[bi++] << 16;
	cmdr->credits += block[bi++] << 8;
	cmdr->credits += block[bi++];
	cmdr->fuel = block[bi++];
	bi++;  
	cmdr->galaxy_number = block[bi++];
	cmdr->front_laser   = block[bi++];
	cmdr->rear_laser    = block[bi++];
	cmdr->left_laser    = block[bi++];
	cmdr->right_laser   = block[bi++];
	
	u8 options_flag = block[bi++];;
	if (options_flag & 1)
	  game_state_flag |= INSTANT_DOCK;
	else
	  game_state_flag &= ~INSTANT_DOCK;	
      	
	if (options_flag & 2) 
	  game_state_flag |= SOLID_SHIPS;
	else
	  game_state_flag &= ~SOLID_SHIPS;
	  
	if (options_flag & 4)
	  game_state_flag |= GBA_PALETTE;
	else
	  game_state_flag &= ~GBA_PALETTE;
	load_ship_palette();

	if (options_flag & 8)
	  game_state_flag |= SHOW_PROGRESS;
	else
	  game_state_flag &= ~SHOW_PROGRESS;

	if (options_flag & 16)
	  game_state_flag |= EASY_MODE;
	else
	  game_state_flag &= ~EASY_MODE;
	  
	if (options_flag & 32)
	  game_state_flag |= DISTANT_DOT;
	else
	  game_state_flag &= ~DISTANT_DOT;

	if (options_flag & 64)
	  game_state_flag |= LUCKY_DOCK_COMP;
	else
	  game_state_flag &= ~LUCKY_DOCK_COMP;
	  
	if (options_flag & 128)
	  game_state_flag |= DOCK_MUSIC;
	else
	  game_state_flag &= ~DOCK_MUSIC;
	
	bi++;
	cmdr->cargo_capacity = block[bi++]-2;

	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
	  cmdr->current_cargo[i] = block[bi++];
    
    	
    cmdr->equipment = block[bi++] << 24;
	cmdr->equipment += block[bi++] << 16;
	cmdr->equipment += block[bi++] << 8;
	cmdr->equipment += block[bi++];	
    
   	cmdr->missiles     = block[bi++];
	cmdr->legal_status = block[bi++];
	
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		cmdr->station_stock[i] = block[bi++];
	
	cmdr->market_rnd  = block[bi++];
	cmdr->score       = block[bi++];
	cmdr->score       += block[bi++]<<8;
	bi++; //0x20

	cmdr->mission3_planet_number = block[bi++];
	cmdr->mission3_galaxy_number = block[bi++];

	// reload the current controls.
	set_default_controls(block[bi],block[bi+1],block[bi+2],block[bi+3]);
        bi+=4;
		
	cmdr->mission_counter = block[bi++];
	
	// a bit of a hack this - get the current time for this commander
	// a bug here is that if you play for  while without saving, 
	// start a new game, then save the commander, the time is larger than you would expect.
	// still "total playing time" is quite a good feature, so I'll leave it in.
    bii = SAVE_OFFSET+(index+1)*SAVE_SIZE - SAVE_BACKSET - 1;

	subseconds = 0;
	seconds = block[bii++];
	minutes = block[bii++];
	hours = block[bii++];
	hours += block[bii++]<<8;

    zoom = 0;
	
    if (index > 7) {
        bi = total_size - QUICK_SAVE_SIZE + SAVE_SIZE;
        quick_saved = block[bi++] = 0;
        zoom = block[bi++];
        laser_temp = block[bi] | (block[bi+1]<<8);
        bi += 2;
        flight_speed = (block[bi] | (block[bi+1]<<8)) - 32768; 
        bi += 2;
        flight_roll = (block[bi] | (block[bi+1]<<8)) - 32768; 
        bi += 2;
        flight_climb = (block[bi] | (block[bi+1]<<8)) - 32768; 
        bi += 2;
        game_state_flag = block[bi] | (block[bi+1]<<8) |
                           (block[bi+2]<<16) | (block[bi+3]<<24);
        bi += 4;
        game_draw_flag = block[bi] | (block[bi+1]<<8) |
                           (block[bi+2]<<16) | (block[bi+3]<<24);
        bi += 4;
        front_shield = block[bi] | (block[bi+1]<<8); 
        bi += 2;
        aft_shield = block[bi] | (block[bi+1]<<8); 
        bi += 2;
        energy = (block[bi] | (block[bi+1]<<8)) - 32768; 
        bi += 2;
        swat_flag = block[bi++];
        missile_target = block[bi++] - 128;
        ecm_active = block[bi++] - 128;

        docked_planet.a = block[bi++];
        docked_planet.b = block[bi++];
        docked_planet.c = block[bi++];
        docked_planet.d = block[bi++];
        docked_planet.e = block[bi++];
        docked_planet.f = block[bi++];
        hyperspace_planet.a = block[bi++];
        hyperspace_planet.b = block[bi++];
        hyperspace_planet.c = block[bi++];
        hyperspace_planet.d = block[bi++];
        hyperspace_planet.e = block[bi++];
        hyperspace_planet.f = block[bi++];

        for (i = 0; i < MAX_UNIV_OBJECTS * sizeof(Object); i++)
            ((unsigned char*)universe)[i] = block[bi++];
        for (i = 0; i <= ELITE_SHIPS.nShipCount; i++)  {
            ship_count[i] = block[bi] | (block[bi+1] << 8);
            bi += 2;
        }

        SaveData(&block[1], total_size-4, "ELT");
        
        generate_planet_data (&current_planet_data, docked_planet);
        generate_stock_market ();
        set_stock_quantities (cmdr->station_stock);

        find_planet_seed(&planet, cmdr->mission3_galaxy_number, cmdr->mission3_planet_number);
        name_planet(mission3_planet_name, planet);
        capitalise_name(mission3_planet_name);        
    }
    else
        *saved_cmdr = *cmdr;
    

	free(block);
	// Enable interrupts
    REG_IME = 0x1;
	return 1;
}


void load_commander_list(void) {
    
  /*
  The data structure is:
  byte 0 success flag - 0 for fail, 1 for OK
  byte 1 8 bit save slots used flag
         - each bit represents that the slot is used
  
  byte 2 commander 1
  byte (n+) commander n...
  
  */

  unsigned char * block;
  
  unsigned short total_size = SAVE_SIZE*MAX_SAVES + 10 + QUICK_SAVE_SIZE;
  block = (unsigned char *) malloc (sizeof(char) * total_size);
  
  unsigned char i,j;
  
  unsigned short start;
  LoadData(block, "ELT");
  
  if (block[0] == 1) {
    // loaded elite data ok
    save_count = block[1];
    quick_saved = block[total_size - QUICK_SAVE_SIZE + SAVE_SIZE];
    
    if (save_count != 0) {
      
      for (i = 0; i < MAX_SAVES; i++) {
        if (save_count & (1<<i)) {
          start = SAVE_OFFSET+i*SAVE_SIZE;
          for (j = 0; j < 7; j++) {
            saved_name[i][j] = block[start+j];
          }
          saved_name[i][7] = 0;
          start = SAVE_OFFSET+(i+1)*SAVE_SIZE - SAVE_BACKSET;
          saved_mins[i] = block[start++];
          saved_hours[i] = block[start++];
          saved_hours[i] += block[start++]<<8;
        }
      }
      
    }    
  }
  else {
    save_count = 0;
  }
  free(block);
}


void init_load_save(unsigned long int ns) {
  unsigned char i;
  next_screen = ns;
  hilite_item = 0;
  hilite_letter = 0;
  for (i = 0; i < 8; i ++) {
    name[i] = cmdr->name[i];
  }
  load_commander_list();
  
  set_message(" ");
  
}


void display_saved_cmdrs(void) {
  unsigned char i;
  char str[30];
  for (i = 0; i < MAX_SAVES; i++) {
    if (hilite_item == i)
      gba_setcolour(GFX_COL_LIGHT_GREEN);
    else
      gba_setcolour(GFX_COL_WHITE);
    sprintf(str, "%d.",i+1);
    gba_print (SAVE_LIST_X_OFFSET, SAVE_LIST_Y_OFFSET+i, str);
    if (save_count & (1<<i)) {
      sprintf(str, "%s (%02dh:%02dm)",saved_name[i], saved_hours[i],saved_mins[i]);
//      gba_print (SAVE_LIST_X_OFFSET+3, SAVE_LIST_Y_OFFSET+i, saved_name[i]);
      gba_print (SAVE_LIST_X_OFFSET+3, SAVE_LIST_Y_OFFSET+i, str);
    }  else {
        gba_print (SAVE_LIST_X_OFFSET+3, SAVE_LIST_Y_OFFSET+i, "--");
    }
  }
}


void load_commander_screen (void) {
  
  current_screen = SCR_LOAD_CMDR;
  gba_clsUnroll();
  
  gba_setcolour(GFX_COL_GOLD);
  gfx_centre_text(0, "LOAD COMMANDER");
  gba_scanline (2, 160-9, 236);
  
  gba_setcolour(GFX_COL_WHITE);
  
  if (save_count > 0)
    gfx_centre_text(2, "Select Commander");
  else
    gfx_centre_text(2, "No Commander saved");
  // display the list of commanders from Save RAM
  
  display_saved_cmdrs();
  
  gba_setcolour( (hilite_item == DEFAULT_JAMESON) ? GFX_COL_LIGHT_GREEN: GFX_COL_WHITE);
  gba_print (SAVE_LIST_X_OFFSET+3, SAVE_LIST_Y_OFFSET+DEFAULT_JAMESON, "Default JAMESON");  
  gba_setcolour( (hilite_item == QUICK_SAVE) ? 
      (quick_saved? GFX_COL_LIGHT_GREEN: GFX_COL_DARK_RED) :
      (quick_saved? GFX_COL_WHITE: GFX_COL_DARK_GREY) );
  gba_print (SAVE_LIST_X_OFFSET+3, SAVE_LIST_Y_OFFSET+QUICK_SAVE, "Quick-Saved Game");  
  
  gba_setcolour (GFX_COL_GOLD);
  gfx_centre_text (18, msg);
}

void save_commander_screen (void) {
  char str[2];
  unsigned char i;
  current_screen = SCR_SAVE_CMDR;  
  gba_clsUnroll();  
  gba_setcolour(GFX_COL_GOLD);
  gfx_centre_text(0, "SAVE COMMANDER");
  gba_scanline (2, 160-9, 236);
    
  // display the list of commanders from Save RAM
  display_saved_cmdrs();

  gba_setcolour(GFX_COL_WHITE);
  gba_print(1,15, "Save as:");
  for (i = 0; i < 8; i++) {
    str[0] = name[i];
    str[1] = 0;
    
    if (hilite_letter == i)
      gba_setcolour(GFX_COL_LIGHT_GREEN);
    else
      gba_setcolour(GFX_COL_WHITE);
    gba_print(10+i,15,str);
  }
  
  if (hilite_letter > 8) {
    gba_print(3,3, "Save in slot:");
  }
  
  gba_setcolour (GFX_COL_GOLD);
  gba_print(3,16, msg);
  
}


void load_save_up(void) {
  
  if (current_screen == SCR_SAVE_CMDR) {
    if (hilite_letter < 8) {
      // change the current letter
      if (name[hilite_letter] == 'Z')
        name[hilite_letter] = 0x20; // space
      else if (name[hilite_letter] == ' ')
        name[hilite_letter] = 'A'; 
      else
        name[hilite_letter]++;
      
    }
    else {
      // change the save slot
      if (hilite_item == 0)
        hilite_item = MAX_LOAD_SLOT-1;
      else
        hilite_item--;

    }
    
  } else {
    if (hilite_item == 0)
      hilite_item = MAX_LOAD_SLOT;
    else
      hilite_item--;
//    hilite_item = !hilite_item;
  }
  
}

void load_save_down(void) {
  if (current_screen == SCR_SAVE_CMDR) {
    if (hilite_letter < 8) {
      // change the current letter
      if (name[hilite_letter] == 'A')
        name[hilite_letter] = 0x20; // space
      else if (name[hilite_letter] == ' ')
        name[hilite_letter] = 'Z'; 
      else
        name[hilite_letter]--; 
      
    }
    else {
      // change the save slot
//      hilite_item = !hilite_item;
      if (hilite_item == MAX_LOAD_SLOT-1)
        hilite_item = 0;
      else
        hilite_item++;
    }
    
  } else {
    if (hilite_item == MAX_LOAD_SLOT)
      hilite_item = 0;
    else
      hilite_item++;
//    hilite_item = !hilite_item;
  }
}

void load_save_right(void) {
  if (current_screen == SCR_SAVE_CMDR) {
    if (hilite_letter < 6)
      hilite_letter++;
  }
  
}
void load_save_left(void) {
  if (current_screen == SCR_SAVE_CMDR) {
    if (hilite_letter > 0 && hilite_letter < 8)
      hilite_letter--;
  }
}

void load_save_accept(void) {
  if (current_screen == SCR_SAVE_CMDR) {
    PlaySoundFX(SND_SELECT);
    if (hilite_letter < 8) {
      hilite_letter = 10;
      hilite_item = 0;
    }
    else {
      // save the commander...
      cmdr->ship_x = docked_planet.d;
  	  cmdr->ship_y = docked_planet.b;
      save_commander_file (hilite_item);
      *saved_cmdr = *cmdr;
  	  set_message("SAVED");
      load_commander_list();
      hilite_letter = 0;
    }
  } else {
    if (hilite_item == DEFAULT_JAMESON) {
      set_cmdr_default();
      restore_saved_commander();
      if (next_screen == SCR_INTRO_TWO) {
        current_screen = SCR_INTRO_TWO;
        return;
      }
      display_commander_status();

    } 
    else if (hilite_item == QUICK_SAVE && quick_saved) {
        if (quick_load()) {
            if (game_state_flag & ESCAPING)
                run_escape_sequence(1);
            else
                display_commander_status();
            return;
        } else {
        // failed to load  
        set_message("FAIL");
        }
    } 
    else  if (save_count & (1<<hilite_item)) {
      if (load_commander_file (hilite_item)) {
        restore_saved_commander();
        if (next_screen == SCR_INTRO_TWO) {
          current_screen = SCR_INTRO_TWO;
          return;
        }
        display_commander_status();
      } else {
        // failed to load  
        set_message("FAIL");
      }
      
    }
    
  }
}

void set_message(char * txt) {
  unsigned char len = strlen(txt);
  unsigned char i;
  for (i = 0; i < 7; i++) {
    if (i < len)
      msg[i] = txt[i];
    else
      msg[i] = ' ';
  }
  msg[7] = 0;
}


void load_save_cancel(void) {
  if (current_screen == SCR_SAVE_CMDR) {
    PlaySoundFX(SND_CANCEL);
    if (hilite_letter > 8) {
      hilite_letter = 0;
      hilite_item = 0;
    }
    
  } else {
    if (next_screen == SCR_INTRO_TWO) {
      current_screen = SCR_INTRO_TWO;
      return;
    }
    display_commander_status();
  }
}


int quick_save(void) {
    
    if (save_commander_file(QUICK_SAVE)) {
        PlaySoundFX(SND_SELECT);
        return 1;
    }
    else
        return 0;
}


int quick_load(void) {
    
    initialise_game();
    if (load_commander_file(QUICK_SAVE)) {
        PlaySoundFX(SND_SELECT);
        return 1;
    }
    else
        return 0;
}

