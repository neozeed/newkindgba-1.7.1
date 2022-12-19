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
 * missions.c
 *
 * Code to handle the special missions.
 */
 
#include <stdio.h>
#include <stdlib.h>
// stdlib should define rand
int rand(void);
#include "elite.h"
#include "gfx.h"
#include "vector.h"
#include "space.h"
#include "planet.h"
#include "missions.h" 
#include "shipdata.h" 
#include "keypad.h" 
#include "space.h" 
#include "swat.h" 
#include "sound.h" 
#include "gbalib_asm.h" 

const unsigned short int mission_req_score[2][3] = {
    {0x0100, 0x0500, 0x06a4}, // 256, 1280, 1700  original vales
    {0x00c0, 0x03c0, 0x0600}  // values for easy ranking mode
};

const static char *mission1_brief_a[2] = {
	"Greetings Commander, I am Captain Curruthers of "
	"Her Majesty's Space Navy and I beg a moment of your "
	"valuable time.  We would like you to do a little job "
	"for us.",
	
	"The ship you see here is a new model, the "
	"Constrictor, equipped with a top secret new shield "
	"generator.  Unfortunately it's been stolen."};

const static char *mission1_brief_b[2] = {
	"It went missing from our ship yard on Xeer five months ago "
	"and was last seen at Reesdice. Your mission, should you decide "
	"to accept it, is to seek and destroy this ship.",
	
	"You are cautioned that only Military Lasers will get through the new "
	"shields and that the Constrictor is fitted with an E.C.M. "
	"System. Good Luck, Commander. ---MESSAGE ENDS."};

const static char *mission1_brief_c[2] = {
	"It went missing from our ship yard on Xeer five months ago "
	"and is believed to have jumped to this galaxy."
	"Your mission should you decide to accept it, is to seek and "
	"destroy this ship." ,
	
	"You are cautioned that only Military Lasers "
	"will get through the new shields and that the Constrictor is "
	"fitted with an E.C.M. System. Good Luck, Commander. ---MESSAGE ENDS."};

const static char *mission1_debrief =
	"There will always be a place for you in Her Majesty's Space Navy. "
	"And maybe sooner than you think... ---MESSAGE ENDS.";

const static char *mission1_pdesc[] =
{
	"THE CONSTRICTOR WAS LAST SEEN AT REESDICE, COMMANDER.",
	"A STRANGE LOOKING SHIP LEFT HERE A WHILE BACK. LOOKED BOUND FOR AREXE.",
	"YEP, AN UNUSUAL NEW SHIP HAD A GALACTIC HYPERDRIVE FITTED HERE, USED IT TOO.",
	"I HEAR A WEIRD LOOKING SHIP WAS SEEN AT ERRIUS.",
	"THIS STRANGE SHIP DEHYPED HERE FROM NOWHERE, SUN SKIMMED AND JUMPED. I HEAR IT WENT TO INBIBE.",
	"ROGUE SHIP WENT FOR ME AT AUSAR. MY LASERS DIDN'T EVEN SCRATCH ITS HULL.",
	"OH DEAR ME YES. A FRIGHTFUL ROGUE WITH WHAT I BELIEVE YOU PEOPLE CALL A LEAD "
		"POSTERIOR SHOT UP LOTS OF THOSE BEASTLY PIRATES AND WENT TO USLERI.",
	"YOU CAN TACKLE THE VICIOUS SCOUNDREL IF YOU LIKE. HE'S AT ORARRA.",
	"THERE'S A REAL DEADLY PIRATE OUT THERE.",
	"BOY ARE YOU IN THE WRONG GALAXY!",
	"COMING SOON: ELITE - DARKNESS FALLS.",				
};

const static char *mission2_brief_a =
	"Attention Commander, I am Captain Fortesque of Her Majesty's Space Navy. "
	"We have need of your services again. If you would be so good as to go to "
	"Ceerdi you will be briefed. If succesful, you will be rewarded. "
	"---MESSAGE ENDS.";
	
const static char *mission2_brief_b[2] = {
	"Good Day Commander. I am Agent Blake of Naval Intelligence. As you know, "
	"the Navy have been keeping the Thargoids off your ass out in deep space "
	"for many years now.", "Well the situation has changed. Our boys are ready "
	"for a push right to the home system of those murderers."};

const static char *mission2_brief_c[2] = {
	"I have obtained the defence plans for their Hive Worlds. The beetles "
	"know we've got something but not what. If I transmit the plans to our "
	"base on Birera they'll intercept the transmission."," I need a ship to "
	"make the run. You're elected. The plans are unipulse coded within "
	"this transmission. You will be paid. Good luck Commander. ---MESSAGE ENDS."};

const static char *mission2_debrief =
	"You have served us well and we shall remember. "
	"We did not expect the Thargoids to find out about you."
	"For the moment please accept this Navy Extra Energy Unit as payment. "
	"---MESSAGE ENDS.";


const static char *mission3_brief_a[2] = {
"We have received a distress signal "
"from %s. The system is being bombarded "
"by large space rocks. All their ships have been "
"destroyed trying to fragment the asteroids.", 
"The asteroids are now dangerously close to the planet "
"- your mission is to go to the system, destroy the asteroids and "
"avert catastrophe. ---MESSAGE ENDS."};

const static char *mission3_brief_b =
	"You must help us Commander! "
	"Please go out there and destroy the asteroids before they reduce our "
	"planet to rubble! "
	"---MESSAGE ENDS.";

const static char *mission3_debrief =
	"Well done - you saved our planet from certain destruction. "
	"Your reward is 6000 Cr. "
	"---MESSAGE ENDS.";

const static char *mission3_pdesc = 
"The planet %s is being bombarded by asteroids.";

const char *mission_planet_desc (struct galaxy_seed planet)
{
	int pnum;

	if (!(game_state_flag & DOCKED))
		return NULL;

	if ((planet.a != docked_planet.a) ||
	    (planet.b != docked_planet.b) ||
	    (planet.c != docked_planet.c) ||
	    (planet.d != docked_planet.d) ||
	    (planet.e != docked_planet.e) ||
	    (planet.f != docked_planet.f))
		return NULL;
	
	pnum = find_planet_number (planet);
	
	if (cmdr->galaxy_number == 0)
	{
		switch (pnum)
		{
			case 150:
				return mission1_pdesc[0];
			
			case 36:
				return mission1_pdesc[1];

			case 28:
				return mission1_pdesc[2];							
		}
	}

	if (cmdr->galaxy_number == 1)
	{
		switch (pnum)
		{
			case 32:
			case 68:
			case 164:
			case 220:
			case 106:
			case 16:
			case 162:
			case 3:
			case 107:
			case 26:
			case 192:
			case 184:
			case 5:
				return mission1_pdesc[3];
		
			case 253:
				return mission1_pdesc[4];
			
			case 79:
				return mission1_pdesc[5];

			case 53:
				return mission1_pdesc[6];							

			case 118:
				return mission1_pdesc[7];							

			case 193:
				return mission1_pdesc[8];							
		}
	}

	if ((cmdr->galaxy_number == 2) && (pnum == 101))
		return mission1_pdesc[9];							
	
	return NULL;
}



const char *mission3_planet_desc (struct galaxy_seed planet)
{
	int pnum;
	
	pnum = find_planet_number (planet);
	
	if ((cmdr->galaxy_number == cmdr->mission3_galaxy_number) && pnum == cmdr->mission3_planet_number)
	{
        static char brief_str[240];
        sprintf(brief_str, mission3_pdesc, mission3_planet_name); 
        return brief_str;
	}

	return NULL;
}


void constrictor_mission_brief (void)
{
	Matrix rotmat;
    
  signed char text_to_show = 0;
    
	cmdr->mission = 1;

	current_screen = SCR_MISSIONS;
    
    clear_universe();
    set_unit_matrix(rotmat);
	add_new_ship (SHIP_CONSTRICTOR, 0, 20<<8, 700<<8, rotmat, -127, -127);
	flight_roll = 0;
	flight_climb = 0;
	flight_speed = 0;

	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
      gfx_centre_text (0, "INCOMING MESSAGE");
      gba_scanline (2, 160-9, 236);
      gfx_centre_text (19, "Press A to continue.");
      update_universe (0);
      
      universe[0].location.z = 700<<8;
      universe[0].location.y = 20<<8;
      UpdateKeys();
      if (KeyPressedNoBounce(KEY_A,4)) {
        text_to_show++;
      }
      gba_setcolour(GFX_COL_WHITE);
      if (text_to_show >= 2 && text_to_show < 4) {
        print_multi_text(1, 10, (cmdr->galaxy_number == 0) ? mission1_brief_b[text_to_show -2] : mission1_brief_c[text_to_show -2]);
      } else if (text_to_show < 2){
        print_multi_text(1, 10,mission1_brief_a[text_to_show]);
      }
      update_screen();
    } while (text_to_show < 4);
    // clean out both buffers
    gba_clsUnroll();
    update_screen();
    gba_clsUnroll();
    update_screen();
}	


void constrictor_mission_debrief (void)
{
	cmdr->mission = 3;
	cmdr->score += 256;
	cmdr->credits += 50000;
	
	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
      gfx_centre_text (0, "INCOMING MESSAGE");
      gba_scanline (2, 160-9, 236);
      gfx_centre_text (19, "Press A to continue.");
      
      
      gfx_centre_text (4, "Congratulations Commander!");
      gba_setcolour(GFX_COL_WHITE);
      print_multi_text(1, 6, mission1_debrief);
      update_screen();
      UpdateKeys();
    } while (!KeyPressedNoBounce(KEY_A,4));
	// clean out both buffers
    gba_clsUnroll();
    update_screen();
    gba_clsUnroll();
    update_screen();
}


void thargoid_mission_first_brief (void)
{
  cmdr->mission = 4;
	do  {
    gba_clsUnroll();
    gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "INCOMING MESSAGE");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    
    gba_setcolour(GFX_COL_WHITE);  
    print_multi_text(1, 3, mission2_brief_a);
    
    
    update_screen();
    UpdateKeys();
  } while (!KeyPressedNoBounce(KEY_A,4));
	// clean out both buffers
  gba_clsUnroll();
  update_screen();
  gba_clsUnroll();
  update_screen();
}


void thargoid_mission_second_brief (void)
{
  cmdr->mission = 5;
	Matrix rotmat;
    
  signed char text_to_show = 0;
  
	current_screen = SCR_MISSIONS;
    
        clear_universe();
        set_unit_matrix(rotmat);
        rotmat[2].z =  1<<FIXEDPOINT;
	add_new_ship (SHIP_THARGOID, 40<<8, 20<<8, 1100<<8, rotmat, -127, -127);
	flight_roll = 0;
	flight_climb = 0;
	flight_speed = 0;

	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "INCOMING MESSAGE");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    update_universe (0);
      
    universe[0].location.z = 1100<<8;
    universe[0].location.y = 20<<8;
    universe[0].location.x = 40<<8;
    UpdateKeys();
    if (KeyPressedNoBounce(KEY_A,4)) {
      text_to_show++;
    }
    
    // display the mission texts 
    gba_setcolour(GFX_COL_WHITE);
    if (text_to_show >= 2 && text_to_show < 4) {
      print_multi_text(1, 11, mission2_brief_c[text_to_show -2]);
    } else if (text_to_show < 2){
      print_multi_text(1, 11,mission2_brief_b[text_to_show]);
    }
    update_screen();
    } while (text_to_show < 4);
    // clean out both buffers
    gba_clsUnroll();
    update_screen();
    gba_clsUnroll();
    update_screen();
  
}


void thargoid_mission_debrief (void)
{
	cmdr->mission = 6;
	cmdr->score += 256;
	cmdr->equipment |= NAVAL_ENERGY_UNIT;
	
	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
      gfx_centre_text (0, "INCOMING MESSAGE");
      gba_scanline (2, 160-9, 236);
      gfx_centre_text (19, "Press A to continue.");
      
      
      gfx_centre_text (4, "Congratulations Commander!");
      gba_setcolour(GFX_COL_WHITE);
      print_multi_text(1, 6, mission2_debrief);
      update_screen();
      UpdateKeys();
    } while (!KeyPressedNoBounce(KEY_A,4));
	// clean out both buffers
    gba_clsUnroll();
    update_screen();
    gba_clsUnroll();
    update_screen();
}


void asteroid_mission_brief (void)
{
  cmdr->mission = 7;
	Matrix rotmat;
    
  signed char text_to_show = 0;
  
	current_screen = SCR_MISSIONS;
    
        clear_universe();
        set_unit_matrix(rotmat);
        rotmat[2].z =  1<<FIXEDPOINT;
	add_new_ship (SHIP_ASTEROID, 40<<8, 20<<8, 1000<<8, rotmat, -127, -127);
	flight_roll = 0;
	flight_climb = 0;
	flight_speed = 0;

	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "INCOMING MESSAGE");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    update_universe (0);
    
    if (text_to_show == 1)
      universe[0].location.z = 800<<8;
    else
      universe[0].location.z = 1000<<8;
      
    universe[0].location.y = 20<<8;
    universe[0].location.x = 40<<8;
    UpdateKeys();
    if (KeyPressedNoBounce(KEY_A,4)) {
      text_to_show++;
      if (text_to_show == 1) {
        clear_universe();
        add_new_ship (SHIP_BOULDER, 40<<8, 20<<8, 800<<8, rotmat, -127, -127);
      }
    }
    
    // display the mission texts 
    gba_setcolour(GFX_COL_WHITE);
    if (text_to_show < 2){
      char brief_str[240];
      sprintf(brief_str, mission3_brief_a[text_to_show], mission3_planet_name); 
      print_multi_text(1, 11, brief_str);
    }
    
    update_screen();
    
  } while (text_to_show < 2);
  // clean out both buffers
  gba_clsUnroll();
  update_screen();
  gba_clsUnroll();
  update_screen();
  
}

void asteroid_mission_second_brief (void)
{
  
	do  {
    gba_clsUnroll();
    gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "INCOMING MESSAGE");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    
    gba_setcolour(GFX_COL_WHITE);  
    print_multi_text(1, 4, mission3_brief_b);
    
    update_screen();
    UpdateKeys();
  } while (!KeyPressedNoBounce(KEY_A,4));
	// clean out both buffers
  gba_clsUnroll();
  update_screen();
  gba_clsUnroll();
  update_screen();
}


void asteroid_mission_debrief (void)
{
  cmdr->mission_counter = 0;
//  cmdr->mission = 8;
  cmdr->credits += 60000;
  
	do  {
    gba_clsUnroll();
    gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "INCOMING MESSAGE");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    
    gba_setcolour(GFX_COL_WHITE);  
    print_multi_text(1, 4, mission3_debrief);
    
    update_screen();
    UpdateKeys();
  } while (!KeyPressedNoBounce(KEY_A,4));
	// clean out both buffers
  gba_clsUnroll();
  update_screen();
  gba_clsUnroll();
  update_screen();
  
  cmdr->mission = 6;
  cmdr->mission3_galaxy_number = (cmdr->mission3_galaxy_number + 1 + rand()%3) & 7;
  cmdr->mission3_planet_number = rand()&255;
  struct galaxy_seed planet;
  find_planet_seed(&planet, cmdr->mission3_galaxy_number, cmdr->mission3_planet_number);
  name_planet(mission3_planet_name, planet);
  capitalise_name(mission3_planet_name);
}


const static char *credits_text[8] = {
	"Congratulations Commander!",
	
	"You have proved yourself as one of the best. A pilot of "
	"the highest quality. You are an Elite combateer.",
	
	"Elite by Bell & Braben",
	
	"The New Kind reverse engineered by Christian Pinder from the original Elite BBC code.",
	
	"GBA port from TNK code by Richard Quirk 2003/2005.    "
	"GFX & maths functions by Pete Horsman. ",
	
	"Music and sound fx by Krawall http://mind.riot.org/krawall/",
	
	"Thanks to Thomas Metz for debugging and improvements.",
	
	"www.geocities.com/quirky_2k1 "
//	"and www.newkind.co.uk !"
	
	};

void show_credits(void) {
  
  cmdr->equipment |= CREDITS_SHOWN;
  Matrix rotmat;
    
  signed char text_to_show = 0;
  
	current_screen = SCR_MISSIONS;
    
  clear_universe();
        set_unit_matrix(rotmat);
        rotmat[2].z =  1<<FIXEDPOINT;
	add_new_ship (SHIP_COBRA3, 0<<8, 20<<8, 800<<8, rotmat, -127, -127);
	flight_roll = 0;
	flight_climb = 0;
	flight_speed = 0;
  PlaySong(0);
	do  {
	  gba_clsUnroll();
	  gba_setcolour(GFX_COL_GOLD);
    gfx_centre_text (0, "CONGRATULATIONS!");
    gba_scanline (2, 160-9, 236);
    gfx_centre_text (19, "Press A to continue.");
    update_universe (0);
    
    universe[0].location.z = 800<<8;
    
    universe[0].location.y = 20<<8;
    universe[0].location.x = 0<<8;
    
    // display the mission texts 
    gba_setcolour(GFX_COL_WHITE);
    print_multi_text(1, 12,credits_text[text_to_show]);
    update_screen();
    
    UpdateKeys();
    if (KeyPressedNoBounce(KEY_A,4)) {
      text_to_show++;
    }
  } while (text_to_show < 8);
  // clean out both buffers
  StopSong();
  gba_clsUnroll();
  update_screen();
  gba_clsUnroll();
  update_screen();
    
  
}


void check_mission_brief (void)
{
    int easy = ((game_state_flag & EASY_MODE) == 0)? 0: 1;

	if ((cmdr->mission == 0) && (cmdr->score >= mission_req_score[easy][0]) && (cmdr->galaxy_number < 2))
	{
		constrictor_mission_brief();
		return;
	}

	if (cmdr->mission == 2)
	{
		constrictor_mission_debrief();
		return;
	}

	if ((cmdr->mission == 3) && (cmdr->score >= mission_req_score[easy][1]) && (cmdr->galaxy_number == 2))
	{
		thargoid_mission_first_brief();
		return;
	}

	if ((cmdr->mission == 4) && (docked_planet.d == 215) && (docked_planet.b == 84))
	{
		thargoid_mission_second_brief();
		return;
	}

	if ((cmdr->mission == 5) && (docked_planet.d == 63) && (docked_planet.b == 72))
	{
		thargoid_mission_debrief();
		return;
	}
    
  if ((cmdr->mission == 6) && (cmdr->score >= mission_req_score[easy][2]) 
      && (cmdr->galaxy_number == cmdr->mission3_galaxy_number)
      && find_planet_number(docked_planet)!=cmdr->mission3_planet_number)
  {
    
    asteroid_mission_brief ();
    return;
  }
   
  if ((cmdr->mission == 7) && (cmdr->galaxy_number == cmdr->mission3_galaxy_number)
   && find_planet_number(docked_planet)==cmdr->mission3_planet_number
   && cmdr->mission_counter < MISSION3_LIMIT)
  {
    asteroid_mission_second_brief ();
    return;
  } else if ((cmdr->mission == 7) && (cmdr->galaxy_number == cmdr->mission3_galaxy_number)
   && find_planet_number(docked_planet)==cmdr->mission3_planet_number
   && cmdr->mission_counter >= MISSION3_LIMIT) {
    asteroid_mission_debrief ();
    return;
  }
  
  int difficulty = (game_state_flag & EASY_MODE)? 0: 1;
  if ((cmdr->score >= ELITE_SCORE(difficulty)) && (!(cmdr->equipment & CREDITS_SHOWN)) ) {
    show_credits();
    return;
  }
  
	/*                      when
	1 constrictor            score >= see above
	2 Thargoid plans         score >= see above, gal==2
	3 Asteroid Bombardment   score >= see above, gal==3
	*/

	
}
