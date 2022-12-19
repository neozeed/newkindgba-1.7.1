// intro.c

#include "gbalib_asm.h"
#include "shipdata.h"
#include "swat.h"
#include "gfx.h"
#include "space.h"
#include "elite.h"
#include "vector.h"
#include "intro.h"
#include "stars.h"


int ship_no;
int show_time;
int direction;

static const unsigned int min_dist[] = {
0,    /*miss*/384,/*cori*/1200, /*esc */300, /*allo*/  0, /*crgo*/200, /*bldr*/  0, /*astr*/520, /*rock*/  0,
			/*shtl*/300, /*trsp*/300, /*cob3*/900, /*pyth*/900, /*boa */500, /*anac*/800, /*herm*/520, /*vipr*/384,
			/*bush*/384, /*sdwn*/384, /*mamb*/384, /*krat*/500, /*addr*/384, /*geck*/384, /*cob1*/384, /*worm*/384,
			/*cob3*/  0, /*asp2*/384, /*pyth*/  0, /*ferd*/384, /*mory*/384, /*thrg*/700, /*thgl*/400, /*cons*/  0,
			/*cugr*/  0,/*dodo*/1200, /*bug */384, /*caml*/  0, /*sht2*/  0, /*salm*/  0, };


void initialise_intro1 (void) {
  gfx_draw_scanner();
	flight_roll = 6;
	clear_universe();
	Matrix intro_ship_matrix;
        set_unit_matrix(intro_ship_matrix);
        //intro_ship_matrix[2].z =  (1<<FIXEDPOINT);
        intro_ship_matrix[2].z =  0x6000;
  
	add_new_ship (SHIP_COBRA3, 0, 0, 4500<<8, intro_ship_matrix, -127, 0);//-127);
}

void initialise_intro2 (void)
{
	ship_no = SHIP_KRAIT;
	show_time = 0;
	direction = 100;
	flight_speed = 3;
	flight_roll = 4;
//	flight_roll = 0;
	flight_climb = 0;
	clear_universe();
	Matrix intro_ship_matrix;
        set_unit_matrix(intro_ship_matrix);
  
	add_new_ship (SHIP_KRAIT, 0, 0, 5000<<8, intro_ship_matrix,  -127, 0);//-127);
}

void update_intro1 (void)
{
	universe[0].location.z -= 100<<8;
//	gba_debug_16_16(0,5,universe[0].location.z);	
	if (universe[0].location.z < (684<<8))
		universe[0].location.z = 684<<8;

	gba_clsUnroll();
	update_universe(0);
	
	gba_setcolour(GFX_COL_WHITE);
	gba_print_masked(5,1,"---- E L I T E ----");
	gba_print_masked(2,12, "(C) D.Braben & I.Bell 1985");
	
	gba_setcolour(GFX_COL_GOLD);
	gba_print_masked(3,10, "Load New Commander (Y/N)?");
	
	update_console();
	
}

void update_intro2 (void)
{
	show_time++;

	if ((show_time >= 380) && (direction < 0))
		direction = -direction;

	universe[0].location.z += (direction<<8);

	if (universe[0].location.z < (min_dist[ship_no]<<8))
		universe[0].location.z = (min_dist[ship_no]<<8);

	if (universe[0].location.z > (4500<<8))
	{
		do
		{
			ship_no++;
			if (ship_no > ELITE_SHIPS.nShipCount)
				ship_no = 1;
		} while (min_dist[ship_no] == 0);

		show_time = 0;
		direction = -100;

		ship_count[universe[0].type] = 0;
		universe[0].type = 0;
		Matrix intro_ship_matrix;
                set_unit_matrix(intro_ship_matrix);
		add_new_ship (ship_no, 0, 0, 4500*256, intro_ship_matrix, -127, 0);//-127);
	}


	gba_clsUnroll();
//	update_starfield();
	update_universe(0);
	
	gba_setcolour(GFX_COL_GOLD);
	gba_print_masked(4,11, "Press Start, Commander.");
	gba_setcolour(GFX_COL_WHITE);
	gfx_centre_text_masked(0, ELITE_SHIPS.pShipDataTable[universe[0].type].pShipData->cShipName);
	
	update_console();

}
