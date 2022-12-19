#include <string.h>
// stdlib should include abs, but is broken in devkitadv
#include "stdlib.h"
unsigned int abs(int);
#include <math.h>
// includes sqrt()

#include "gba.h"
#include "gbalib_asm.h"
#include "sleep.h"
#include "sound.h"
#include "gfx.h"
#include "elite.h"
#include "vector.h"
#include "space.h"
#include "stars.h"
#include "interrupts.h"
#include "threed.h"
#include "dma.h"
#include "shipdata.h"
#include "swat.h"
#include "intro.h"
#include "random.h"
#include "missions.h"
#include "keypad.h"
#include "docked.h"
#include "options.h"
#include "main.h"
#include "file.h"
#include "menu.h"
#include "pilot.h"

#include "registers.h"
// TM added before first compilation succeeded

#include "krawall.h"

#define NULL    0
#define FRAME_RATE  4
#define MESSAGE_BUFFER_SIZE 8
//#define TIME_ENABLE 0x80
//#define TIME_IRQ_ENABLE 0x40
// frame rate is actually 256/(FRAME_RATE + 1)

//int fps_count;

extern unsigned char _binary_img_master_pal_start[];

unsigned char test_thruster;
unsigned char rolling;
unsigned char climbing;
unsigned char message_count;
signed char draw_lasers;

unsigned char short_range_flag = 1;
// indicates if L button was pressed on short range chart (1)
// or on galactic chart (0), needed to return properly from
// data on planet screen when L button pressed
// see also function l_pressed()

unsigned char subseconds; 
unsigned char seconds;    
unsigned char minutes;    
unsigned short hours;     

unsigned char zoom, zoom_changed=0;

int scanner_rim_y[SCANNER_MAX_X+1];

char message_string[MESSAGE_BUFFER_SIZE][30];
unsigned char last_message_out = 0, last_message_in = 0;


//void FastIntDivide (signed long Numer, signed long Denom, signed long *Result, signed long *Remainder);
void initialise_gba(void);
void WaitForVsync(void);
void VblankIntr(void);
//void Timer2Intr(void);
void JoypadIntr(void);
void run_first_intro_screen (void);
void run_second_intro_screen (void);
void run_game_over_screen(void);
void run_escape_sequence (int skip_self_destruct_sequence);
void handle_flight_keys (void);
void arrow_up (void);
void arrow_down (void);
void arrow_left (void);
void arrow_right (void);
void a_pressed (void);
void b_pressed (void);
void l_pressed (void);
void r_pressed (void);
void select_pressed (void);
void draw_laser_sights(void);
void move_cross (int dx, int dy);
void display_break_pattern (void);
void auto_dock (void);
void info_message (char *message);
void update_screen(void);
void draw_cross (unsigned char cx, unsigned char cy);

void switch_zoom(void);
void decrease_zoom(void);

void small_handle_flight_keys(void);

//////////////////////////////////////////////
//Object oShip;
//Object oPlanet;

//IntrTable for crt0
void (*IntrTable[])() = {
	VblankIntr, // v-blank
	0, // h-blank
	0, // vcount
	0,//Timer0UpdateSound, // timer0
	kradInterrupt, //NMOD_Timer1iRQ, // timer1
	0,//Timer2Intr, // timer2
	0, // timer3
	0, // serial
	0, // dma0
	0, // dma1
	0, // dma2
	0, // dma3
        JoypadIntr, // key
	0  // cart
};

char __EH_FRAME_BEGIN__[] = {};
// to resolve link-errors w/o crtbegin.o (-nostartfiles), which we don't really need in C

void initialise_gba(void) {
    // initialise gba specific things

    gba_setmode(0x04);
    gba_initbank();
    load_ship_palette();
    init_load_save(0);
    
    InitKeys();
    initialise_universe();
    frames = 0;
    LoadOBJPalette(0);
    REG_DISPCNT |= 0x40|0x1000;
    gba_clsUnroll();
    
    // build up scanner rim table
    int x;    
    for (x=0; x<=SCANNER_MAX_X; x++)
        scanner_rim_y[x] = (int)(SCANNER_MAX_Y * sqrt(1 - ((double)x / SCANNER_MAX_X)*((double)x / SCANNER_MAX_X)));

}


void intr_main(void);

#ifdef DevkitARM
int main(void) 
#else
int AgbMain(void) 
#endif
{
  
  initialise_gba();

  REG_IME = 0x00;                       // Disable interrupts

  // allocate ram for commander structs
  initialise_cmdr();
    
  create_new_stars();

  REG_IME = 0x00;                       // Disable interrupts
  REG_IE |= INT_VBLANK;//| INT_TIMMER2;
//  REG_TM2CNT = 0x0 | 0x80 | 0x40; // FREQ_64, TIME_ENABLE, TIME_IRQ_ENABLE
//  fps_count = 0;
  
  REG_DISPSTAT = BIT03;             // Enable Display V-Blank IRQ also.
  REG_IME = 0x1;                    // Enable interrupts
  kragInit( KRAG_INIT_STEREO ); // init krawall


  test_thruster = 0;
  update_console();
  set_default_controls(OPTION_R_DEF, OPTION_L_DEF, OPTION_B_DEF,OPTION_S_DEF);
  gfx_draw_scanner();
  
  set_default_options();
  // game_state_flag |= SOLID_SHIPS;
  
  while (1) {
        
        initialise_game();

        reset_weapons();

        current_screen = SCR_FRONT_VIEW;
        run_first_intro_screen();
        gfx_remove_scanner ();
        while (current_screen == SCR_LOAD_CMDR) {
          load_commander_screen();

          update_screen();
          small_handle_flight_keys ();
        }        
        
        if (current_screen == SCR_INTRO_TWO) {
          gfx_draw_scanner ();
          run_second_intro_screen();
          dock_player ();
        }
        if (!(game_state_flag & ESCAPING) && 
            current_screen != SCR_BREAK_PATTERN_ESC_POD) {
            display_commander_status ();
            WaitForVsync();
            menu_set_docked();
            gba_swapbank();
            display_commander_status ();
        }

        while(!(game_state_flag & GAME_OVER)) {
            
//            Testing message buffer
//            char str[20]; int i;
//            sprintf(str, "%d/%d", last_message_out, last_message_in);
//            gba_print (0,0,str);
//            for (i=0; i<=3; i++) 
//                gba_print (0,i+1, &message_string[i][0]);
            
            
            if (!(game_state_flag & GAME_PAUSED))
              update_screen();

            rolling = 0;
            climbing = 0;

            handle_flight_keys ();

            if (game_state_flag & GAME_PAUSED)
                continue;

            if (message_count > 0)
                message_count--;

            if (!(game_state_flag & DOCKED) && !(game_state_flag & WITCHSPACE)
            && (game_state_flag & WARP_JUMP) ) {
              jump_warp();
            }

            if (!climbing)
            {
                if (flight_climb > 0)
                    decrease_flight_climb();

                if (flight_climb < 0)
                    increase_flight_climb();
            }

            if (!rolling)
            {
                if (flight_roll > 0)
                    decrease_flight_roll();

                if (flight_roll < 0)
                    increase_flight_roll();
          }

          if (!(game_state_flag & DOCKED))
            {
                if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
                    (current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW) ||
                    (current_screen == SCR_INTRO_ONE) || (current_screen == SCR_INTRO_TWO) ||
                    (current_screen == SCR_GAME_OVER))
                {
                    gba_clsUnroll();
//                    update_starfield();
                }

                if (game_state_flag & AUTO_PILOT)
                {
                    auto_dock();
                  if ((mcount & 127) == 0)
                      info_message ("Docking Computer On");
                }

                update_universe (1);

                if (game_state_flag & DOCKED)
                {
                    continue;
                }

                if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
                    (current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
                {
                    if (draw_lasers > 0)
                    {
                        if (enemy_hit) {
                            gba_setcolour(GFX_COL_ORANGE);
                            gfx_draw_spot(laser_x + rand255()%7 - 3, laser_y + rand255()%9 - 4);
                            gfx_draw_spot(laser_x + rand255()%7 - 3, laser_y + rand255()%9 - 4);
                            gba_setcolour(GFX_COL_GOLD);
                            gfx_draw_spot(laser_x + rand255()%5 - 2, laser_y + rand255()%5 - 2);
                            gba_setcolour(GFX_COL_WHITE);
                            gfx_draw_spot(laser_x, laser_y);
                            enemy_hit = 0;
                        }
                        draw_laser_lines();
                        draw_lasers--;


                    }

                    draw_laser_sights();
                }

                if ((message_count == 0) && (last_message_in != last_message_out)) {
                        // show next message in queue
                        last_message_out = (last_message_out+1) % MESSAGE_BUFFER_SIZE;
                        message_count = (last_message_in == last_message_out)? 37: 24;
                        PlaySoundFX(SND_BEEP);
                    }
                        

                if (message_count > 0) {
                  gba_setcolour(GFX_COL_WHITE);
                  gfx_centre_text_masked(12,&message_string[last_message_out][0]);
                }

                if (game_state_flag & HYPER_READY && ((mcount & 3) == 0)) {
                  countdown_hyperspace();
                }

                mcount--;


                if ((mcount & 7) == 0)
                    regenerate_shields();

                if ((mcount & 31) == 10)
                {
                    if (energy < 50)
                    {
                      info_message ("ENERGY LOW");
                    }
                  update_altitude();
                }

                if ((mcount & 31) == 20)
                  update_cabin_temp();

                if ((mcount == 0) && (!(game_state_flag & WITCHSPACE)))
                  random_encounter();

                cool_laser();
                time_ecm();

                update_console();
            }

            if (cross_timer > 0)
            {
                cross_timer--;
                if (cross_timer == 0)
                {
                    show_distance_to_planet();
                }
            }

            switch (current_screen) {
              case SCR_GALACTIC_CHART:  display_galactic_chart(); break;
              case SCR_SHORT_RANGE: display_short_range_chart();break;
              case SCR_CMDR_STATUS:  display_commander_status(); break;
              case SCR_MARKET_PRICES:  display_market_prices(); break;
              case SCR_INVENTORY:  display_inventory(); break;
              case SCR_EQUIP_SHIP:  equip_ship(); break;
              case SCR_PLANET_DATA: display_data_on_planet (); break;
              case SCR_BREAK_PATTERN: 
              case SCR_BREAK_PATTERN_ESC_POD: display_break_pattern(); break;
              case SCR_FRONT_VIEW: break;

              case SCR_SETTINGS: game_settings_screen(); break;
              case SCR_CONFIG: game_config_screen(); break;
              case SCR_OPTIONS: display_options(); break;
              case SCR_LOAD_CMDR: load_commander_screen(); break;
              case SCR_SAVE_CMDR: save_commander_screen(); break;
              case SCR_QUICK_SAVE: quick_save_screen (); break;
              case SCR_QUIT: quit_screen (); break;
              case SCR_EXIT: exit_screen (); break;
           }

           if (!(game_state_flag & DOCKED) && (game_state_flag & HYPER_READY))
           {
                display_hyper_status();
           }


          check_menu();
        }
        if (!(game_state_flag & FINISH))
          run_game_over_screen();
        
        // make sure we aren't showing any of the extra stuff for in the intros
        gfx_remove_big_e ();
        gfx_remove_big_s ();
        gfx_remove_big_c ();
      }

}




//void Timer2Intr(void) {
//  fps_count++;
//  REG_IF = INT_TIMMER2;
//}




void run_first_intro_screen (void)
{
    current_screen = SCR_INTRO_ONE;
    initialise_intro1();

    PlaySong(0);

    while (1)
    {
        update_intro1();

        update_screen();

        UpdateKeys();
        if (KeyPressedNoBounce(KEY_A,2)) {

          init_load_save (SCR_INTRO_TWO);

          load_commander_screen();
          break;
        }

        if (KeyPressedNoBounce(KEY_B,2)) {
          current_screen = SCR_INTRO_TWO;
          break;
        }

    }
    StopSong();


}


void run_second_intro_screen (void)
{
    current_screen = SCR_INTRO_TWO;

    PlaySong(1);

    initialise_intro2();

    for (;;)
    {
        update_intro2();

        update_screen();

        UpdateKeys();
        if (KeyPressedNoBounce(KEY_START,2))
          break;
    }
    StopSong();

}


// this is used in between the intros - it is a reduced keyboard routine
// mainly so you can't show the menu in this bit
void small_handle_flight_keys(void) {
  UpdateKeys();
  if (KeyPressedNoBounce(KEY_UP,2))
      load_save_up();

  if (KeyPressedNoBounce(KEY_DOWN,2))
      load_save_down();

  if (KeyPressedNoBounce(KEY_A,2))
      load_save_accept();

  if (KeyPressedNoBounce(KEY_B,2))
      load_save_cancel();
}

// anti-bounce for the laser when menu option is selected
int menu_selected;

void handle_flight_keys (void)
{
  UpdateKeys();

  if (game_state_flag & GAME_PAUSED) {
    if (KeyPressed(KEY_START))
      pause_game();
    return;
  }
  
  if (KeyPressedNoBounce(KEY_START,3)) {
      draw_menu();
    }

  if (KeyPressed(KEY_SELECT)) {
      select_pressed();
    }

  if (KeyPressed(KEY_UP)) {
      arrow_up();
    }

  if (KeyPressed(KEY_DOWN)) {
      arrow_down();
  }

  if (KeyPressed(KEY_LEFT)) {
    arrow_left();
    }

  if (KeyPressed(KEY_RIGHT)) {
      arrow_right();
  }

  // A and B buttons moved downwards, necessary to avoid laser shot when zooming

  if (KeyPressed(KEY_L)) {
    l_pressed();
  }

  if (KeyPressed(KEY_R)) {
    r_pressed();
  }

  if (KeyPressed(KEY_A)) {
    a_pressed();
  } else {
    menu_selected = 0;
  }
  
  if (KeyPressed(KEY_B)) {
    b_pressed();
  }

  if (!KeyPressed(KEY_A) && !KeyPressed(KEY_B)) 
    zoom_changed = 0;
  
}


void arrow_up (void)
{
  if ((game_draw_flag & MENU_DRAWN)) {
    menu_up();
  }
  else {
    switch (current_screen)
    {
        case SCR_MARKET_PRICES:
          if (KeyPressedNoBounce(KEY_UP,10))
              select_previous_stock();
            break;

        case SCR_EQUIP_SHIP:
          if (KeyPressedNoBounce(KEY_UP,10))
              equip_up();
            break;

        case SCR_OPTIONS:
          if (KeyPressedNoBounce(KEY_UP,4))
              select_previous_option();
            break;

        case SCR_SETTINGS:
            if (KeyPressedNoBounce(KEY_UP,4))
              select_up_setting();
            break;

        case SCR_CONFIG:
            if (KeyPressedNoBounce(KEY_UP,4))
              select_up_config();
            break;


        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_UP,4))
            load_save_up();
          break;

        case SCR_SHORT_RANGE:
        case SCR_GALACTIC_CHART:
          if (!(game_state_flag & FIND_PLANET))
            move_cross (0, -1);
          else if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_UP,2)) {
            up_find_char();
          }
          break;

        case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:

            if (flight_climb > 0)
                flight_climb = 0;
            else
            {
                decrease_flight_climb();
            }
            climbing = 1;

            break;
    }
  }

}

void arrow_down (void)
{
  if ((game_draw_flag & MENU_DRAWN)) {
    menu_down();
  } else {
    switch (current_screen)
    {
        case SCR_MARKET_PRICES:
          if (KeyPressedNoBounce(KEY_DOWN,10))
              select_next_stock();
            break;

        case SCR_EQUIP_SHIP:
          if (KeyPressedNoBounce(KEY_DOWN,10))
              equip_down();
            break;

        case SCR_OPTIONS:
          if (KeyPressedNoBounce(KEY_DOWN,4))
              select_next_option();
            break;

        case SCR_SETTINGS:
        if (KeyPressedNoBounce(KEY_DOWN,4))
              select_down_setting();
            break;

        case SCR_CONFIG:
            if (KeyPressedNoBounce(KEY_DOWN,4))
              select_down_config();
            break;

        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_DOWN,4))
            load_save_down();
          break;

        case SCR_SHORT_RANGE:
        case SCR_GALACTIC_CHART:
          if (!(game_state_flag & FIND_PLANET))
              move_cross (0, 1);
            else if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_DOWN,2)) {
              down_find_char();
            }

            break;

        case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:

              if (flight_climb < 0)
                  flight_climb = 0;
              else
              {
                  increase_flight_climb();
              }
              climbing = 1;
              break;

    }
  }

}


void arrow_right (void)
{
  if ((game_draw_flag & MENU_DRAWN)) {
    menu_right();
  } else {
    switch (current_screen)
    {
        case SCR_MARKET_PRICES:
            if (KeyPressedNoBounce(KEY_RIGHT,5)) {
            buy_stock();
            
            }
            break;

        case SCR_EQUIP_SHIP:
          if (KeyPressedNoBounce(KEY_RIGHT,10))
              equip_right();
            break;

        case SCR_SETTINGS:
        if (KeyPressedNoBounce(KEY_RIGHT,4))
              select_right_setting();
            break;

        case SCR_CONFIG:
            if (KeyPressedNoBounce(KEY_RIGHT,4))
              select_config_toggle();
            break;

        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_RIGHT,4))
            load_save_right();
          break;


        case SCR_SHORT_RANGE:
        case SCR_GALACTIC_CHART:
          if (!(game_state_flag & FIND_PLANET))
            move_cross(1, 0);
          else if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_RIGHT,2)) {
//            right_find_char();
          }
          break;

        case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
            if (flight_roll > 0)
                flight_roll = 0;
            else
            {
                decrease_flight_roll();
                decrease_flight_roll();
                rolling = 1;
            }
            break;
    }
  }

}


void arrow_left (void)
{
  if ((game_draw_flag & MENU_DRAWN)) {
    menu_left();
  } else {
    switch (current_screen)
    {
        case SCR_MARKET_PRICES:
            if (KeyPressedNoBounce(KEY_LEFT,5))
                sell_stock();
            break;

      case SCR_EQUIP_SHIP:
          if (KeyPressedNoBounce(KEY_LEFT,10))
              equip_left();
            break;

        case SCR_SETTINGS:
          if (KeyPressedNoBounce(KEY_LEFT,4))
              select_left_setting();
            break;

        case SCR_CONFIG:
            if (KeyPressedNoBounce(KEY_LEFT,4))
              select_config_toggle();
            break;


        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_LEFT,4))
            load_save_left();
          break;

        case SCR_SHORT_RANGE:
        case SCR_GALACTIC_CHART:
          if (!(game_state_flag & FIND_PLANET))
            move_cross(-1, 0);
          else if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_LEFT,2)) {
//            left_find_char();
          }
          break;

        case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
            if (flight_roll < 0)
                flight_roll = 0;
            else
            {
                increase_flight_roll();
                increase_flight_roll();
                rolling = 1;
            }
            break;
    }
  }

}


void a_pressed (void) {
  if (game_draw_flag & MENU_DRAWN) {
   menu_select();
   menu_selected = 1;
  } else {
    if (menu_selected)
      return;
    switch (current_screen)
    {
        case SCR_GALACTIC_CHART:
          if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_A,5))
            add_find_char ();
//          else 
//            galactic_a_pressed();
          break;

        case SCR_EQUIP_SHIP:
          if (KeyPressedNoBounce(KEY_A,10))
              buy_equip();
            break;

        case SCR_OPTIONS:
          if (KeyPressedNoBounce(KEY_A,4))
              do_option();
            break;

        case SCR_CONFIG:
          if (KeyPressedNoBounce(KEY_A,4))
              config_a_button();
            break;

        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_A,4))
            load_save_accept();
          break;

        case SCR_QUIT:
          if (KeyPressedNoBounce(KEY_A,4))
              do_quit();
          break;

        case SCR_QUICK_SAVE:
          if (KeyPressedNoBounce(KEY_A,4)) {
              if (quick_save())
                  do_quit();
              else
                  do_cancel_quit();
          }
          break;

        case SCR_EXIT:
          if (KeyPressedNoBounce(KEY_A,4))
              do_exit();
            break;

        case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
        if (zoom_changed == 0)
            draw_lasers = fire_laser();
        break;
    }

  }
}

void b_pressed (void) {
  if (game_draw_flag & MENU_DRAWN) {
    draw_menu();
  } else {
    switch (current_screen)
    {
        case SCR_GALACTIC_CHART:
        case SCR_SHORT_RANGE:
          if (!(game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_B,10))
            move_cursor_to_origin();
          else if ((game_state_flag & FIND_PLANET) && KeyPressedNoBounce(KEY_B,10)) {
            delete_find_char();
          }
          break;

        case SCR_OPTIONS:

            break;

        case SCR_SETTINGS:

        break;


        case SCR_LOAD_CMDR:
        case SCR_SAVE_CMDR:
          if (KeyPressedNoBounce(KEY_B,4))
            load_save_cancel();
          break;

        case SCR_QUIT:
        case SCR_EXIT:
        case SCR_QUICK_SAVE:    
          if (KeyPressedNoBounce(KEY_B,4))
              do_cancel_quit();
            break;

      case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
        if (zoom_changed == 0)
          if ( (n_B_Button_Del == 0) || KeyPressedNoBounce(KEY_B,n_B_Button_Del))
            (*p_B_Button_Func)();
        
        break;
    }

  }
}

void l_pressed (void) {
  if (game_draw_flag & MENU_DRAWN) {
    menu_left();
  } else {
    switch (current_screen)
    {
      
      case SCR_SHORT_RANGE:
      case SCR_GALACTIC_CHART:
        if (KeyPressedNoBounce(KEY_L,2)) {
          display_data_on_planet();
        }
        break;
      
      
      case SCR_PLANET_DATA:
        if (KeyPressedNoBounce(KEY_L,2)) {
          if (short_range_flag == 1) {
            set_cross_short_range();
            display_short_range_chart();
          }
          else {
            display_galactic_chart();
          }
        }
        break;

      case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
        if ( (n_L_Button_Del == 0) || KeyPressedNoBounce(KEY_L,n_L_Button_Del))
          (*p_L_Button_Func)();

        break;
    }

  }
}

void r_pressed (void) {
  if (game_draw_flag & MENU_DRAWN) {
    menu_right();
  } else {
    switch (current_screen)
    {

      case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
        if ( (n_R_Button_Del == 0) || KeyPressedNoBounce(KEY_R,n_R_Button_Del))
          (*p_R_Button_Func)();

        break;
    }

  }
}

void select_pressed (void) {

  if (game_draw_flag & MENU_DRAWN) {
    return;
  } else {
    switch (current_screen)
    {

      case SCR_FRONT_VIEW:
        case SCR_REAR_VIEW:
        case SCR_RIGHT_VIEW:
        case SCR_LEFT_VIEW:
        if ( (n_S_Button_Del == 0) || KeyPressedNoBounce(KEY_SELECT,n_S_Button_Del))
          (*p_S_Button_Func)();

        break;
    }

  }

}


void draw_laser_sights(void)
{
    unsigned char laser = 0;
    gba_setcolour(GFX_COL_WHITE);
    switch (current_screen)
    {
        case SCR_FRONT_VIEW:
        case SCR_BREAK_PATTERN:
          gfx_centre_text_masked(1,"Front View");
            laser = cmdr->front_laser;
            break;

        case SCR_REAR_VIEW:
          gfx_centre_text_masked(1,"Rear View");
            laser = cmdr->rear_laser;
            break;

        case SCR_LEFT_VIEW:
          gfx_centre_text_masked(1,"Left View");
            laser = cmdr->left_laser;
            break;

        case SCR_RIGHT_VIEW:
          gfx_centre_text_masked(1,"Right View");
            laser = cmdr->right_laser;
            break;
    }

  gba_setcolour(GFX_COL_GOLD);
    if (laser)
    {
      unsigned char centerx = DownRight.x>>1;
      unsigned char centery = DownRight.y>>1;


      gba_horizline(centerx-19,centery,10);
      gba_horizline(centerx+9,centery,10);
      gba_horizline(centerx-19,centery-1,10);
      gba_horizline(centerx+9,centery-1,10);

      gba_vertline(centerx,centery-19,10);
      gba_vertline(centerx,centery+9,10);
    gba_vertline(centerx+1,centery-19,10);
      gba_vertline(centerx+1,centery+9,10);

    }
}



void move_cross (int dx, int dy)
{
    cross_timer = 5;

    if (current_screen == SCR_SHORT_RANGE)
    {
        cross_x += (dx << 2);
        cross_y += (dy << 2);
        
        if (cross_x < -docked_planet.d<<2)
            cross_x = -docked_planet.d<<2;

        if (cross_x > ((256-docked_planet.d)<<2) - 1)
            cross_x = ((256-docked_planet.d)<<2) - 1;

        if (cross_y < -docked_planet.b<<1)
            cross_y = -docked_planet.b<<1;

        if (cross_y > ((256-docked_planet.b)<<1) - 1)
            cross_y = ((256-docked_planet.b)<<1) - 1;
 
        return;
    }

    if (current_screen == SCR_GALACTIC_CHART)
    {
        cross_x += (dx << 1);
        cross_y += (dy << 1);

        if (cross_x < 0)
            cross_x = 0;

        if (cross_x > times_GALAXY_RESIZE_FACTOR(128)-1)
            cross_x = times_GALAXY_RESIZE_FACTOR(128)-1;

        if (cross_y < 0)
            cross_y = 0;

        if (cross_y > times_GALAXY_RESIZE_FACTOR(64)-1)
            cross_y = times_GALAXY_RESIZE_FACTOR(64)-1;
    
    }
}


/*
 *  Draw a break pattern (for launching, docking and hyperspacing).
 *  draws 6 circles at a distance "z+n*40"
 *  every frame the circles get closer to give a tunnel impression
 *  lots of hardcoded values, but it looks ok in action
 */

#define MAX_BREAK_CIRCLES 6
void display_break_pattern (void)
{

  if (current_screen == SCR_BREAK_PATTERN_ESC_POD)
      remove_all_scanner_gfx();    
  else {
      gfx_draw_scanner();
  }
    
  signed short circle_z[10];
  unsigned char i,j;
  
  for (i = 0; i < MAX_BREAK_CIRCLES; i++) {
    circle_z[i] = 100+ i*40;
  }

  unsigned short radius;
  gba_clsUnroll();
  i = 0;
  while (i == 0) {
    draw_laser_sights();
    if (current_screen != SCR_BREAK_PATTERN_ESC_POD)
        update_console();
    update_screen();
    gba_clsUnroll();
    gba_setcolour(GFX_COL_WHITE);
    for (j = 0; j < MAX_BREAK_CIRCLES; j++) {
      if (circle_z[j] > 0) {
        radius = gba_div(1000,circle_z[j]);

        gba_circle_clipped (radius, DownRight.x>>1, DownRight.y>>1, 0);
        circle_z[j]-=10;
      } else if (j == (MAX_BREAK_CIRCLES - 1) ) {
        i = 1;
      }
    }
  }

  if (game_state_flag & DOCKED)
  {
    gfx_remove_scanner();
    check_mission_brief();
    display_commander_status();
  }
  else {
      current_screen = SCR_FRONT_VIEW;
      if (game_state_flag & WITCHSPACE)
        info_message("Drive Malfunction!");
  }

}


/*
 * Draw the game over sequence.
 */

void run_game_over_screen(void)
{
    unsigned short i;
    signed char newship;
    Matrix rotmat;
    unsigned char type;

    current_screen = SCR_GAME_OVER;
    PlaySoundFX(SND_GAMEOVER);

    flight_speed = 6;
    flight_roll = 0;
    flight_climb = 0;
    clear_universe();

    set_unit_matrix(rotmat);

    newship = add_new_ship (SHIP_COBRA3, 0, 0, -400<<8, rotmat, 0, 0);
    universe[newship].flags |= FLG_DEAD;

    for (i = 0; i < 5; i++)
    {
        type = (rand255() & 1) ? SHIP_CARGO : SHIP_ALLOY;
        newship = add_new_ship (type, ((rand255() & 31) - 16)<<8,
                                ((rand255() & 31) - 16)<<8, -400<<8, rotmat, 0, 0);
        universe[newship].rotz = ((rand255() * 2) & 255) - 128;
        universe[newship].rotx = ((rand255() * 2) & 255) - 128;
        universe[newship].velocity = rand255() & 15;
    }


    for (i = 0; i < 100; i++)
    {
        gba_clsUnroll();
//        update_starfield();
        update_universe(1);
        gba_setcolour(GFX_COL_GOLD);
        gfx_centre_text_masked(5,"GAME OVER");
        update_screen();
    }
}


void auto_dock (void)
{

    Object ship;

    ship.flags = 0;
    ship.location.x = 0;
    ship.location.y = 0;
    ship.location.z = 0;

    set_unit_matrix(ship.rotmat);

    ship.rotmat[2].z = 1<<FIXEDPOINT;
    ship.rotmat[0].x = -1<<FIXEDPOINT;
    ship.type = -96;
    ship.velocity = flight_speed;
    ship.acceleration = 0;
    ship.bravery = 0;
    ship.rotz = 0;
    ship.rotx = 0;

    auto_pilot_ship (&ship);

    if (ship.velocity > 22)
        flight_speed = 22;
    else
        flight_speed = ship.velocity;

    if (ship.acceleration > 0)
    {
        flight_speed++;
        if (flight_speed > 22)
            flight_speed = 22;
    }

    if (ship.acceleration < 0)
    {
        flight_speed--;
        if (flight_speed < 1)
            flight_speed = 1;
    }

    if (ship.rotx == 0)
        flight_climb = 0;

    if (ship.rotx < 0)
    {
        increase_flight_climb();

        if (ship.rotx < -1)
            increase_flight_climb();
    }

    if (ship.rotx > 0)
    {
        decrease_flight_climb();

        if (ship.rotx > 1)
            decrease_flight_climb();
    }

    if (ship.rotz == 127)
        flight_roll = -14;
    else
    {
        if (ship.rotz == 0)
            flight_roll = 0;

        if (ship.rotz > 0)
        {
            increase_flight_roll();

            if (ship.rotz > 1)
                increase_flight_roll();
        }

        if (ship.rotz < 0)
        {
            decrease_flight_roll();

            if (ship.rotz < -1)
                decrease_flight_roll();
        }
    }

}

void info_message (char *message)
{
    // place message in queue
    unsigned char new_message;
    new_message = (last_message_in+1) % MESSAGE_BUFFER_SIZE;
    if (new_message != last_message_out) {
        strcpy (&message_string[last_message_in=new_message][0], message);
        if (message_count > 24) message_count = 24;
    }

}


void update_screen(void) 
{
  // make the thrusters flash a bit
  test_thruster++;
  gba_setpalette( 6, 20+test_thruster, 6, 4);
  if (test_thruster >= 8)
    test_thruster = 0;

//  // debug - show frames taken
//  gba_setcolour(GFX_COL_WHITE);
//  gba_debug_16_16(0,0,fps_count);
//  gba_debug_16_16(0,1,minutes);
//  gba_debug_16_16(0,2,hours);  
  
//  fps_count = 0;
  
  while(frames < FRAME_RATE) {
    WaitForVsync();
//    gba_vsync();
  }

  frames = 0;
  gba_swapbank();
}

void speed_selection (void) {
  // the speed control toggle, for use with up and down
  if (KeyPressed(KEY_UP)) {
    increase_flight_climb();
    speed_up();
  }

  else if (KeyPressed(KEY_DOWN)) {

    decrease_flight_climb();
    slow_down();
  }

}

void view_selection (void) {
  // the view selection toggle, for use with up and down left and right
  if (KeyPressed(KEY_UP)) {
    increase_flight_climb();
    display_front_view();
  } else if (KeyPressed(KEY_DOWN)) {
    display_rear_view();
    decrease_flight_climb();
  } else if (KeyPressed(KEY_LEFT)) {
    decrease_flight_roll();
    decrease_flight_roll();
    display_left_view();
  } else if (KeyPressed(KEY_RIGHT)) {
    increase_flight_roll();
    increase_flight_roll();
    display_right_view();
  }
  if (KeyPressed(KEY_A) && zoom_changed==0) {        
    switch_zoom();
    zoom_changed = 1;
  }
  if (KeyPressed(KEY_B) && zoom_changed==0) {        
    decrease_zoom();
    zoom_changed = 1;
  }

}


void speed_up(void) {
  if (flight_speed < myship.max_speed)
    flight_speed++;
}

void slow_down(void) {
  if (flight_speed > 1)
    flight_speed--;
}

// launch the escape pod
void run_escape_sequence (int skip_self_destruct_sequence)
{
	unsigned char i;
	signed char newship;
	Matrix rotmat;

	current_screen = SCR_ESCAPE_POD;
    game_state_flag |= ESCAPING;

	// remove the menu draw scanner
	if (game_draw_flag & MENU_DRAWN)
        draw_menu();
	remove_all_scanner_gfx();
//	gfx_draw_scanner();
    gba_clsUnroll();

    if (!skip_self_destruct_sequence) {

    	flight_speed = 1;
    	flight_roll = 0;
    	flight_climb = 0;
    
        set_unit_matrix(rotmat);
        rotmat[2].z =  1<<FIXEDPOINT;
    
    	newship = add_new_ship (SHIP_COBRA3, 0, 0, 200<<8, rotmat, -127, -127);
    	universe[newship].velocity = 7;
        PlaySoundFX(SND_LAUNCH);
    	for (i = 0; i < 90; i++)
    	{
    		if (i == 40)
    		{
    			universe[newship].flags |= FLG_DEAD;
    			PlaySoundFX(SND_EXPLODE);
    		}
    
        gba_clsUnroll();
    //	update_starfield();
    	update_universe(1);
    
    	universe[newship].location.x = 0;
    	universe[newship].location.y = 0;
    	universe[newship].location.z += 2<<8;
    
        gba_setcolour(GFX_COL_WHITE);
        gfx_centre_text_masked (10,"Escape pod launched");
        gfx_centre_text_masked (11,"Ship auto-destruct initiated");
    
    	//update_console();
    	update_screen();
    	}
    
    }
    
    while (!(game_state_flag & DOCKED) &&
           !(game_state_flag & GAME_OVER))
//	while ((ship_count[SHIP_CORIOLIS] == 0) &&
//		   (ship_count[SHIP_DODEC] == 0))
	{
        if (!(game_state_flag & GAME_PAUSED))
		    update_screen();

        handle_flight_keys();
        
        if (game_state_flag & GAME_PAUSED)
            continue;
        
        // Drive back climbing and rolling
        // otherwise we will not get the same behaviour
        // of the auto pilot, which is optimised for these
        // back-driving forces of the climb and roll levers
        // and the resulting damping in their movements.
        // In the main loop, these 'forces' were originally
        // introduced to drive back climbing and rolling when
        // the d-pad is not touched.
        if (flight_climb > 0)
            decrease_flight_climb();
        if (flight_climb < 0)
            increase_flight_climb();
        if (flight_roll > 0)
            decrease_flight_roll();
        if (flight_roll < 0)
            increase_flight_roll();

		auto_dock();

		if ((abs(flight_roll) < 3) && (abs(flight_climb) < 3))
		{
			for (i = 0; i < MAX_UNIV_OBJECTS; i++)
			{
				if (universe[i].type != 0)
					universe[i].location.z -= 1500;
			}

	    }
	  
//	    game_draw_flag |= WARP_STARS;
	    gba_clsUnroll();

   		update_universe(1);

        switch (current_screen) {
              case SCR_SETTINGS: game_settings_screen(); break;
              case SCR_CONFIG: game_config_screen(); break;
              case SCR_OPTIONS: display_options(); break;
              case SCR_LOAD_CMDR: load_commander_screen(); break;
              case SCR_SAVE_CMDR: save_commander_screen(); break;
              case SCR_QUICK_SAVE: quick_save_screen (); break;
              case SCR_QUIT: quit_screen (); break;
              case SCR_EXIT: exit_screen (); break;
        }

        check_menu();
//		update_starfield();
//		update_console();
	}

	if (!(game_state_flag & GAME_OVER))
        abandon_ship();
	if (energy < 0)
	   energy = 0;
    game_state_flag &= ~ESCAPING;

}


// pauses the game, shows a message which is in a different location depending on the screen
// stops screen updating in the main loop
void pause_game(void) {

  if (game_state_flag & GAME_PAUSED)
    game_state_flag &= ~GAME_PAUSED;
  else {
    game_state_flag |= GAME_PAUSED;

    if ((current_screen == SCR_FRONT_VIEW) || (current_screen == SCR_REAR_VIEW) ||
			(current_screen == SCR_LEFT_VIEW) || (current_screen == SCR_RIGHT_VIEW))
		{
      gba_setcolour(GFX_COL_WHITE);
      gfx_centre_text_masked(9,"** PAUSED **");
      gfx_centre_text_masked(11,"START TO RESUME");
      gfx_clear_status_area();
      update_console();
    } else {
      gfx_clear_status_area();
      gba_setcolour(GFX_COL_WHITE);
      gfx_centre_text_masked(16,"** PAUSED **");
      gfx_centre_text_masked(18,"START TO RESUME");
    }
    
    update_screen();

  }

}

void switch_zoom(void)
{
  zoom++;
  zoom %= 5;
}


void decrease_zoom(void)
{
    zoom = (zoom + 5 - 1) % 5;
}


void WaitForVsync(void)
{
  asm volatile("swi 0x05");
//  asm volatile("swi 0x50000"); 
}

// not used, but works ok in testing - doubt it's much faster than Pete's gba_div routine?
// Do an integer division using GBA BIOS fast division SWI code.
//  by Jeff F., 2002-Apr-27

// GCC 2.9/3.0 Version

//void FastIntDivide (signed long Numer, signed long Denom, signed long *Result, signed long *Remainder)
// {
//asm volatile
//  (
//  " mov   r0,%2   \n"
//  " mov   r1,%3   \n"
//  " swi   0x60000       \n"   //NOTE!!!!! Put 6 here for Thumb C Compiler mode.
//  " ldr   r2,%0   \n"   //          Put 0x60000 there for ARM C Compiler mode.
//  " str   r0,[r2] \n"
//  " ldr   r2,%1   \n"
//  " str   r1,[r2] \n"
//  : "=m" (Result), "=m" (Remainder) // Outputs
//  : "r" (Numer), "r" (Denom)        // Inputs
//  : "r0","r1","r2","r3"             // Regs crushed & smushed
//  );
//}
