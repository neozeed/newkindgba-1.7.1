#include <string.h>
#include "options.h"
#include "gfx.h"
#include "gba.h"
#include "sleep.h"
#include "elite.h"
#include "docked.h"
#include "space.h"
#include "pilot.h"
#include "swat.h"
#include "main.h"
#include "file.h"
#include "reset.h"
#include "gbalib_asm.h"

#include "registers.h"
// added before first compilation succeeded, TM

#define NUM_OPTIONS 7
#define NUM_SETTINGS 4
#define NUM_KEYS 26
#define NUM_CONFIGS 9

void (*p_B_Button_Func)();
void (*p_L_Button_Func)();
void (*p_R_Button_Func)();
void (*p_S_Button_Func)();

unsigned char n_R_Button_Del;
unsigned char n_L_Button_Del;
unsigned char n_B_Button_Del;
unsigned char n_S_Button_Del;



void display_option_item (unsigned char i);
void display_setting_item (unsigned char item);
void display_config_item (unsigned char item);

////////////////////////////////
unsigned char hilite_option[NUM_SETTINGS];
unsigned char hilite_item;

const static char *info_txt = "Select the controls for use in flight.";

struct option
{
	char *text;
	unsigned char docked_only;
};

const static struct option option_list[NUM_OPTIONS] =
{
	{"Save Commander",     1},
	{"Load Commander",     1},
	{"Quick Save",         0},
	{"Settings",           0},
	{"Sleep Mode",         0},
	{"Quit Game",          0},
    {"Exit",               0}	
};

const static char * key_list[NUM_SETTINGS] = {
  "R    :", "L    :", "B    :", "Sel. :"};

const static char * config_list[NUM_CONFIGS] = {
  "Ranking:", "Rating Gauge:", "Instant Dock:", "Docking Computer", "Docking Music:",
  "Solid Ships:", "GBA Palette:", "Distant Objects:", "Control Settings"};

typedef struct MenuItem
{
    char * cName;
    void (*pFunc)();
    unsigned char delay;
} MENUITEM;

static const MENUITEM m0  = {"Front View",&display_front_view,2};
static const MENUITEM m1  = {"Rear View",&display_rear_view,2};
static const MENUITEM m2  = {"Left View",&display_left_view,2};
static const MENUITEM m3  = {"Right View",&display_right_view,2};

static const MENUITEM m4  = {"Galactic Chart",&display_galactic_chart,2};
static const MENUITEM m5  = {"Short Range Chart",&display_short_range_chart,2};
static const MENUITEM m6  = {"Information on planet",&display_data_on_planet,2};
static const MENUITEM m7  = {"Prices",&display_market_prices,2};

static const MENUITEM m8  = {"Commander information",&display_commander_status,2};
static const MENUITEM m9  = {"Inventory",&display_inventory,2};

static const MENUITEM m10  = {"Options",&display_options,2};
static const MENUITEM m11  = {"Switch Zoom",&switch_zoom,2};
static const MENUITEM m12  = {"Docking Computer",&docking_computer_selected,2};
static const MENUITEM m13  = {"E.C.M.",&ecm_selected,2};
static const MENUITEM m14  = {"Hyperspace",&start_hyperspace,2};               
static const MENUITEM m15  = {"Warp Jump",&jump_warp,2};
static const MENUITEM m16  = {"Target/Unarm Missile",&target_missile_toggle,2};                           
static const MENUITEM m17  = {"Fire Missile",&fire_missile,2};                             
static const MENUITEM m18  = {"Galactic Hyperspace",&start_galactic_hyperspace,2};
static const MENUITEM m19  = {"Energy Bomb",&energy_bomb_selected,2};
static const MENUITEM m20  = {"Launch Escape Pod",&escape_capsule_selected,2};
static const MENUITEM m21  = {"Pause",&pause_game,2};
static const MENUITEM m22  = {"Speed up",&speed_up,0};
static const MENUITEM m23  = {"Slow down",&slow_down,0};
static const MENUITEM m24  = {"Speed toggle",&speed_selection,0};
static const MENUITEM m25  = {"View toggle",&view_selection,0};

static const MENUITEM *OptionItems[]= {
  &m0,
  &m1,
  &m2,
  &m3,
  &m4,
  &m5,
  &m6,
  &m7,
  &m8, 
  &m9,      
  &m10,
  &m11,
  &m12,
  &m13,
  &m14,
  &m15,
  &m16,
  &m17,
  &m18,
  &m19,
  &m20,
  &m21,
  &m22,
  &m23,
  &m24,
  &m25,
};

void select_left_setting (void){
  // should select the previous option  
  if (hilite_option[hilite_item] > 0) {
    hilite_option[hilite_item]--;
    switch (hilite_item) {
      case 0:p_R_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_R_Button_Del=OptionItems[hilite_option[hilite_item]]->delay; break;// R
      case 1:p_L_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_L_Button_Del=OptionItems[hilite_option[hilite_item]]->delay; break;// L 
      case 2:p_B_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_B_Button_Del=OptionItems[hilite_option[hilite_item]]->delay; break;// B
      case 3:p_S_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_S_Button_Del=OptionItems[hilite_option[hilite_item]]->delay; break;// select
    }
    
  }
  
}

void select_right_setting (void){
  // should select the next option
  if (hilite_option[hilite_item] < NUM_KEYS-1) {
    hilite_option[hilite_item]++;
    switch (hilite_item) {
      case 0:p_R_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_R_Button_Del=OptionItems[hilite_option[hilite_item]]->delay;break;// R
      case 1:p_L_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_L_Button_Del=OptionItems[hilite_option[hilite_item]]->delay;break;// L 
      case 2:p_B_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_B_Button_Del=OptionItems[hilite_option[hilite_item]]->delay;break;// B
      case 3:p_S_Button_Func = OptionItems[hilite_option[hilite_item]]->pFunc; n_S_Button_Del=OptionItems[hilite_option[hilite_item]]->delay;break;// select
    }
  }
  
}


void select_up_setting (void)
{
  if (hilite_item > 0)
    hilite_item --;
}


void select_down_setting (void)
{
  if (hilite_item < (NUM_SETTINGS - 1))
	  hilite_item ++;
}



void game_settings_screen (void)
{
	unsigned char i;

	current_screen = SCR_SETTINGS;

	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "CONTROL SETTINGS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
    print_multi_text(2,3,info_txt);
//    if (game_draw_flag & MENU_DRAWN)
//	    hilite_item = 0;
    
	for (i = 0; i < NUM_SETTINGS; i++)	{
		display_setting_item (i);
	}
	
}


void display_setting_item (unsigned char item)
{
	if (hilite_item == item)
	  gba_setcolour(GFX_COL_LIGHT_GREEN);
	else
	  gba_setcolour(GFX_COL_WHITE);
	
	gba_print(1, 6+item*2, key_list[item]);
	gba_print(7, 6+item*2, OptionItems[hilite_option[item]]->cName);
	
}



void select_previous_option (void)
{
	if (hilite_item > 0)
		hilite_item --;
}

void select_next_option (void)
{
	if (hilite_item < (NUM_OPTIONS - 1))
	  hilite_item ++;
}


void do_option (void)
{
	if (!(game_state_flag & DOCKED) && option_list[hilite_item].docked_only)
		return;

	switch (hilite_item)
	{
		case 0:
			init_load_save (SCR_CMDR_STATUS);
			save_commander_screen();
			break;
			
		case 1:
            init_load_save (SCR_CMDR_STATUS);
			load_commander_screen();
//			display_commander_status();
			break;
		
		case 2:
            init_load_save (SCR_CMDR_STATUS);
            quick_save_screen();
			break;
		
		case 3:
            hilite_item = 0;
			game_config_screen();
			break;
		
		case 4:
			Sleep_Mode(1);
			break;
		
		case 5:
			quit_screen();
			break;
			
		case 6:
            exit_screen();
            break;
	}
}

extern const char * const version_string;
void display_options (void)
{
	unsigned char i;

	current_screen = SCR_OPTIONS;
	
  if (game_draw_flag & RADAR_DRAWN) {
    remove_all_scanner_gfx();
  }
	
	
	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "GAME OPTIONS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
        char str[30];
        str[0]=0;
        strcat(str,"GBA Version: ");
        strcat(str,version_string);
        
	gfx_centre_text (16, str);
	gfx_centre_text (17, "By Christian Pinder 2001");
	gfx_centre_text (18, "GBA port by Quirky 2003/2005");
	
	if (game_draw_flag & MENU_DRAWN)
	  hilite_item = 0;
	
	for (i = 0; i < NUM_OPTIONS; i++)
	  display_option_item (i);

}


void display_option_item (unsigned char i)
{
	unsigned char col;
	
	if (hilite_item == i)
	  col = (!(game_state_flag & DOCKED) && option_list[i].docked_only) ? GFX_COL_DARK_RED : GFX_COL_LIGHT_GREEN;
	else
	  col = (!(game_state_flag & DOCKED) && option_list[i].docked_only) ? GFX_COL_DARK_GREY : GFX_COL_WHITE;
	
    gba_setcolour(col);
	gfx_centre_text (2+i*2, option_list[i].text);
}

void quit_screen (void)
{
	current_screen = SCR_QUIT;

	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "GAME OPTIONS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
    gfx_centre_text (8, "QUIT GAME (Y/N)?");
    
}

void quick_save_screen (void)
{
	current_screen = SCR_QUICK_SAVE;

	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "GAME OPTIONS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
    gfx_centre_text (8, "QUICK SAVE AND QUIT (Y/N)?");
    
}

void exit_screen (void)
{
	current_screen = SCR_EXIT;

	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "GAME OPTIONS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
    gfx_centre_text (8, "EXIT PROGRAM (Y/N)?");
    
}

void do_quit(void) {
  game_state_flag |= FINISH;
  game_state_flag |= GAME_OVER;
}

void do_cancel_quit(void) {
  hilite_item = 0;
  current_screen = SCR_OPTIONS;
}

void do_exit(void) {
  game_state_flag |= FINISH;
  game_state_flag |= GAME_OVER;
  REG_IME = 0x00;
  //RESET_GBA;
  reset_gba();
}

void set_default_controls(unsigned char opt1, unsigned char opt2, unsigned char opt3, unsigned char opt4 ) {
  unsigned char i;
  // check that the options are not going to crash something
  if (opt1 > NUM_KEYS) 
    opt1 = OPTION_R_DEF;
  if (opt2 > NUM_KEYS) 
    opt2 = OPTION_L_DEF;
  if (opt3 > NUM_KEYS) 
    opt3 = OPTION_B_DEF;
  if (opt3 > NUM_KEYS) 
    opt3 = OPTION_S_DEF ; 
  
  hilite_option[0] = opt1; // R default is speed up
  hilite_option[1] = opt2; // L default is speed down
  hilite_option[2] = opt3; // B default is fire missile
  hilite_option[3] = opt4; // Select default is target missile
  for (i = 0; i < 4; i++) {
    switch (i) {
      case 0:p_R_Button_Func = OptionItems[hilite_option[i]]->pFunc; n_R_Button_Del=OptionItems[hilite_option[i]]->delay;break;// R
      case 1:p_L_Button_Func = OptionItems[hilite_option[i]]->pFunc; n_L_Button_Del=OptionItems[hilite_option[i]]->delay;break;// L 
      case 2:p_B_Button_Func = OptionItems[hilite_option[i]]->pFunc; n_B_Button_Del=OptionItems[hilite_option[i]]->delay;break;// B
      case 3:p_S_Button_Func = OptionItems[hilite_option[i]]->pFunc; n_S_Button_Del=OptionItems[hilite_option[i]]->delay;break;// select  
    }
  }
  
  
}

void set_default_options(void)
{
    game_state_flag &= ~INSTANT_DOCK & ~GBA_PALETTE & ~SHOW_PROGRESS
                      & ~EASY_MODE & ~DISTANT_DOT;
    game_state_flag |= LUCKY_DOCK_COMP | SOLID_SHIPS | DOCK_MUSIC;
    load_ship_palette();
}

void game_config_screen (void)
{
	unsigned char i;

	current_screen = SCR_CONFIG;

	gba_clsUnroll();
	gba_setcolour(GFX_COL_GOLD);
	gfx_centre_text (0, "SETTINGS");
	gba_scanline (2, 160-9, 236);
	gba_setcolour(GFX_COL_WHITE);
  if (game_draw_flag & MENU_DRAWN)
	  hilite_item = 0;
    
	for (i = 0; i < NUM_CONFIGS; i++)	{
		display_config_item (i);
	}

	gba_setcolour(GFX_COL_WHITE);

	if (game_state_flag & EASY_MODE) {
	  gba_print(18,  2, "EASY"); 
	} else {
	  gba_print(18,  2, "ORIGINAL"); 
	}
	
	if (game_state_flag & SHOW_PROGRESS) {
	  gba_print(18,  4, "ON"); 
	} else {
	  gba_print(18,  4, "OFF"); 
	}
	
	if (game_state_flag & INSTANT_DOCK) {
	  gba_print(18,  6, "ON"); 
	} else {
	  gba_print(18,  6, "OFF"); 
	}
	
	if (game_state_flag & LUCKY_DOCK_COMP) {
      gba_print(18,  8, "NEVER FAILS");
    } else {
      gba_print(18,  8, "MAY FAIL");
    }

	if (game_state_flag & DOCK_MUSIC) {
	  gba_print(18, 10, "ON"); 
	} else {
	  gba_print(18, 10, "OFF"); 
	}
	
	if (game_state_flag & SOLID_SHIPS) {
	  gba_print(18, 12, "ON"); 
	} else {
	  gba_print(18, 12, "OFF"); 
	}
	
	if (game_state_flag & GBA_PALETTE) {
	  gba_print(18, 14, "ON"); 
	} else {
	  gba_print(18, 14, "OFF"); 
	}
	
	if (game_state_flag & DISTANT_DOT) {
      gba_print(18,  16, "DOTS");
    } else {
      gba_print(18,  16, "CROSSES");
    }
	
	
}

void display_config_item (unsigned char i)
{
	unsigned char col;
	
	if (hilite_item == i)
	  col = GFX_COL_LIGHT_GREEN;
	else
	  col = GFX_COL_WHITE;
	
  gba_setcolour(col);
  gba_print(1, 2+i*2, config_list[i]);
}


void config_a_button(void) {
    switch (hilite_item) {
        case NUM_CONFIGS-1:
            hilite_item = 0;
            game_settings_screen();
            break;
        default:
            select_config_toggle();
            break;
    }
}

void select_up_config (void)
{
  if (hilite_item > 0)
    hilite_item --;
}


void select_down_config (void)
{
  if (hilite_item < (NUM_CONFIGS - 1))
	  hilite_item ++;
}

void select_config_toggle (void) {
    switch(hilite_item) {
      case 0: 
        if (game_state_flag & EASY_MODE)
          game_state_flag &= ~EASY_MODE;
        else
          game_state_flag |= EASY_MODE;
        break;
      case 1:
        if (game_state_flag & SHOW_PROGRESS)
          game_state_flag &= ~SHOW_PROGRESS;
        else
          game_state_flag |= SHOW_PROGRESS;
        break;
      case 2:
        if (game_state_flag & INSTANT_DOCK)
          game_state_flag &= ~INSTANT_DOCK;
        else
          game_state_flag |= INSTANT_DOCK;
        break;  
      case 3:
        if (game_state_flag & LUCKY_DOCK_COMP)
          game_state_flag &= ~LUCKY_DOCK_COMP;
        else
          game_state_flag |= LUCKY_DOCK_COMP;
        break;  
      case 4:
        if (game_state_flag & DOCK_MUSIC)
          game_state_flag &= ~DOCK_MUSIC;
        else
          game_state_flag |= DOCK_MUSIC;
        break;  
      case 5:
        if (game_state_flag & SOLID_SHIPS)
          game_state_flag &= ~SOLID_SHIPS;
        else
          game_state_flag |= SOLID_SHIPS;
        break;  
      case 6:
        if (game_state_flag & GBA_PALETTE)
          game_state_flag &= ~GBA_PALETTE;
        else
          game_state_flag |= GBA_PALETTE;
        load_ship_palette();
        break;
      case 7:
        if (game_state_flag & DISTANT_DOT)
          game_state_flag &= ~DISTANT_DOT;
        else
          game_state_flag |= DISTANT_DOT;
        break;
  }
}


void get_current_controls(unsigned char * con1, unsigned char * con2, unsigned char * con3, unsigned char * con4) {
  
  // return the currently selected control options
  *con1 = hilite_option[0];
  *con2 = hilite_option[1];
  *con3 = hilite_option[2];
  *con4 = hilite_option[3];
  
}

