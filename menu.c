#include "keypad.h"
#include "gfx.h"
#include "elite.h"
#include "docked.h"
#include "space.h"
#include "pilot.h"
#include "swat.h"
#include "options.h"
#include "sound.h"
#include "main.h"
#include "gbalib_asm.h"

unsigned char selected_menu_item;


// the address of the Function to call when item is chosen
typedef struct tagMenuItem
{
    char * cName;
    void (*pFunc)();
} MENUITEM;

static const MENUITEM m0_0  = {"Launch",&launch_player};
static const MENUITEM m1_0  = {" ",0};
static const MENUITEM m2_0  = {"Find Planet",&display_find_planet};
static const MENUITEM m3_0  = {"Equip Ship",&equip_ship};

static const MENUITEM m0_1  = {"Front View",&display_front_view};
static const MENUITEM m1_1  = {"Rear View",&display_rear_view};
static const MENUITEM m2_1  = {"Left View",&display_left_view};
static const MENUITEM m3_1  = {"Right View",&display_right_view};

static const MENUITEM m4  = {"Galactic Chart",&display_galactic_chart};
static const MENUITEM m5  = {"Short Range Chart",&display_short_range_chart};
static const MENUITEM m6  = {"Information on Planet",&display_data_on_planet};
static const MENUITEM m7_0  = {"Buy and Sell",&display_market_prices};
static const MENUITEM m7_1  = {"Prices",&display_market_prices};

static const MENUITEM m8  = {"Commander Information",&display_commander_status};
static const MENUITEM m9  = {"Inventory",&display_inventory};

static const MENUITEM m10  = {"Options",&display_options};
static const MENUITEM m11  = {"Switch Zoom",&switch_zoom};
static const MENUITEM m12  = {"Docking Computer",&docking_computer_selected};
static const MENUITEM m13  = {"E.C.M.",&ecm_selected};
static const MENUITEM m14  = {"Hyperspace",&start_hyperspace};               
static const MENUITEM m15  = {"Warp Jump",&jump_warp};
static const MENUITEM m16  = {"Target/Unarm Missile",&target_missile_toggle};                           
static const MENUITEM m17  = {"Fire Missile",&fire_missile};                             
static const MENUITEM m18  = {"Galactic Hyperspace",&start_galactic_hyperspace};
static const MENUITEM m19  = {"Energy Bomb",&energy_bomb_selected};
static const MENUITEM m20  = {"Launch Escape Pod",&escape_capsule_selected};
static const MENUITEM m21  = {"Pause",&pause_game};

static const MENUITEM *MenuItemsDocked[]= {
  &m0_0,
  &m1_0,
  &m2_0,
  &m3_0,
  &m4,
  &m5,
  &m6,
  &m7_0,
  &m8, 
  &m9,      
  &m10,
};

static const MENUITEM *MenuItems[]= {
  &m0_1,
  &m1_1,
  &m2_1,
  &m3_1,
  &m4,
  &m5,
  &m6,
  &m7_1,
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
};

void draw_menu(void) {
  gba_clsUnroll();
//  selected_menu_item = 0;
  if (!(game_draw_flag & MENU_DRAWN)) {
    if (game_state_flag & DOCKED) {
      gfx_draw_menu(1);
    } else {
      gfx_draw_menu(0);
    }
    // turn off warp jump..
    if (game_state_flag & WARP_JUMP) 
	  game_state_flag &= ~WARP_JUMP;
    
    // turn off docking computer
    if (game_state_flag & AUTO_PILOT) {
	  disengage_auto_pilot();
	}
	
    gfx_remove_big_s();
    gfx_remove_big_e();
    
    game_draw_flag |= MENU_DRAWN;
    LoadOBJPalette(1);
    game_state_flag &= ~FIND_PLANET;
  } else {
    game_draw_flag &= ~MENU_DRAWN;
    gfx_remove_menu();
    if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
			(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
		{
      gfx_draw_scanner();
    }
    
  }
  
}

void check_menu(void) {
  if (game_draw_flag & MENU_DRAWN) {
    // clear the bottom section of screen and show which is selected
    gfx_clear_status_area();
    // get the current selected item and highlight it
    gba_setcolour(GFX_COL_WHITE);
    if ((game_state_flag & DOCKED)) {
      // use the docked menu..  
      gfx_centre_text(19,MenuItemsDocked[selected_menu_item]->cName);
    } else {
      gfx_centre_text(19,MenuItems[selected_menu_item]->cName);
    }
    
    // selected_menu_item 0 is 4, 4
    unsigned char x = 11;
    unsigned char y = 3;
    if ((game_state_flag & DOCKED)) {
      y = 12;
    }
    
    if (selected_menu_item < 11)
     x = x + selected_menu_item * 20;
    else {
      x = x + (selected_menu_item-11) * 20;
      y = 24;
    }
    
    gfx_draw_selection_square(x,y);
  }

}

void menu_up(void) {
  if (!(game_state_flag & DOCKED)) {
    if (KeyPressedNoBounce(KEY_UP,2)) {
      if (selected_menu_item<11)
        selected_menu_item+=11;
      else
        selected_menu_item-=11;
    } 
    
  }
  
}
void menu_down(void) {
  if (!(game_state_flag & DOCKED)) {
    if (KeyPressedNoBounce(KEY_DOWN,2)) {
      if (selected_menu_item<11)
        selected_menu_item+=11;
      else
        selected_menu_item-=11;
    }
    
  }
  
}

void menu_right(void) {
  if (KeyPressedNoBounce(KEY_RIGHT,2)) {
    if (selected_menu_item == 10) {
      selected_menu_item = 0;
    }
    else if (selected_menu_item == 21) {
      selected_menu_item = 11;
    } else {
      selected_menu_item++;
    }
    
  }  
}

void menu_left(void) {
  if (KeyPressedNoBounce(KEY_LEFT,2)) {
    if (selected_menu_item == 0) {
      selected_menu_item = 10;
    }
    else if (selected_menu_item == 11) {
      selected_menu_item = 21;
    } else {
      selected_menu_item--;
    }
    
  }
  
}

void menu_select(void) {
  if((game_state_flag & ESCAPING) &&
     MenuItems[selected_menu_item]->pFunc != &display_options &&
     MenuItems[selected_menu_item]->pFunc != &pause_game &&
     MenuItems[selected_menu_item]->pFunc != &display_front_view)
      return;
 
  if (game_draw_flag & MENU_DRAWN) {
    if ((game_state_flag & DOCKED) && MenuItemsDocked[selected_menu_item]->pFunc != 0) {
      // use the docked menu..  
      MenuItemsDocked[selected_menu_item]->pFunc();
      if (game_draw_flag & MENU_DRAWN)
        draw_menu();
    } else if (MenuItems[selected_menu_item]->pFunc != 0){
      MenuItems[selected_menu_item]->pFunc();
      if (game_draw_flag & MENU_DRAWN)
        draw_menu();
      if (!( (current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
					(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))) {
      	gfx_remove_scanner();
      	gfx_remove_big_s();
      	gfx_remove_big_e();
      }
    }    
  }
}


// immediately after docking this should be called to reset the selection
// or anything else that might need doing
void menu_set_docked(void) {
  // select commander info by default
  selected_menu_item = 8;
}



