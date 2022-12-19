// stdlib should include abs, but is broken in devkitadv
//#include <stdlib.h>
unsigned int abs(int);
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "space.h"
#include "shipdata.h"
#include "swat.h"
#include "elite.h"
#include "gfx.h"
#include "sound.h"
#include "trade.h"
#include "missions.h"
#include "random.h"
#include "gbalib_asm.h"
#include "pilot.h"
#include "space.h"

signed char ecm_active;
signed char missile_target;
unsigned char laser_counter;
unsigned char laser;
unsigned long int swat_flag;
unsigned char laser_x;
unsigned char laser_y;
unsigned char enemy_hit;

unsigned char in_target (signed char type, signed long int x, signed long int y, signed long int z);
void launch_enemy (unsigned char un, signed char type,  unsigned short int flags, unsigned short int bravery);
void launch_shuttle (void);
void missile_tactics (unsigned char un);
void make_angry (unsigned char un);
void launch_loot (unsigned char un, short int loot);
void track_object (Object *ship, signed long int direction, Vertex nvec);
void create_thargoid (void);
signed char create_other_ship (signed short type);
void create_cougar (void);
void create_trader (void);
void create_lone_hunter (void);

void check_for_asteroids (void);
void check_for_cops (void);
void check_for_others (void);
void check_for_asteroid_mission(void);
void check_missiles (unsigned char un);

/////////////////////////////////////////////////////////////////////

void reset_weapons (void){
	laser_temp = 0;
	laser_counter = 0;
	laser = 0;
	swat_flag = 0;
	missile_target = MISSILE_UNARMED;
}


void clear_universe (void)
{
	int i;

	for (i = 0; i < MAX_UNIV_OBJECTS; i++) {
		universe[i].type = 0;
		universe[i].flags = 0;
	}
	

	for (i = 0; i <= ELITE_SHIPS.nShipCount; i++)
		ship_count[i] = 0;

	if (swat_flag & IN_BATTLE)
		swat_flag &= ~IN_BATTLE;
}


void explode_object (unsigned char un) {
  cmdr->score++;
  if ((cmdr->score & 255) == 0)
    info_message ("Right On Commander!");
//	snd_play_sample (SND_EXPLODE);
	PlaySoundFX(SND_EXPLODE);
	universe[un].flags |= FLG_DEAD;
	
	if (universe[un].type == SHIP_CONSTRICTOR)
		cmdr->mission = 2;
  
  
  // the asteroid mission...
  if (cmdr->mission == 7 && universe[un].type == SHIP_ASTEROID) {
    if (cmdr->mission_counter < MISSION3_LIMIT)
      cmdr->mission_counter++;
    else if (cmdr->mission_counter == MISSION3_LIMIT) {
      info_message ("Asteroids threat destroyed!");  
      cmdr->mission_counter++;
    }
  }
  
}


void check_missiles (unsigned char un)
{
	unsigned char i;
	
	if (missile_target == un)
	{
		missile_target = MISSILE_UNARMED;
		info_message ("Target Lost");
	}

	for (i = 0; i < MAX_UNIV_OBJECTS; i++)
	{
		if ((universe[i].type == SHIP_MISSILE) && (universe[i].target == un))
			universe[i].flags |= FLG_DEAD;
	}
}


void remove_ship (unsigned char un)
{
	signed short int type;
	Matrix rotmat;
	signed long int px,py,pz;

	type = universe[un].type;

	if (type == 0)
		return;

	if (type > 0)
		ship_count[type]--;

	universe[un].type = 0;

	check_missiles (un);

	if ((type == SHIP_CORIOLIS) || (type == SHIP_DODEC))
	{
        set_unit_matrix(rotmat);
		px = universe[un].location.x>>8;
		py = universe[un].location.y>>8;
		pz = universe[un].location.z>>8;

		py &= 0xFFFF;
		py |= (((docked_planet.d & 7) | 1) << 16);
//
		add_new_ship (SHIP_SUN, px, py<<8, pz<<8, rotmat, 0, 0);
	}
}

unsigned char fire_laser (void)
{
	if ((laser_counter == 0) && (laser_temp < 242))
	{
		switch (current_screen)
		{
			case SCR_FRONT_VIEW:
				laser = cmdr->front_laser;
				break;

			case SCR_REAR_VIEW:
				laser = cmdr->rear_laser;
				break;

			case SCR_RIGHT_VIEW:
				laser = cmdr->right_laser;
				break;

			case SCR_LEFT_VIEW:
				laser = cmdr->left_laser;
				break;

			default:
				laser = 0;
		}

		if (laser != 0)
		{
			laser_counter = (laser > 127) ? 0 : (laser & 0xFA);
			laser &= 127;
//			laser2 = laser;
      PlaySoundFX(SND_PULSE);
//			snd_play_sample (SND_PULSE);
			laser_temp += 8;
			if (energy > 1)
				energy--;

			laser_x = ((rand255() & 3) + (DownRight.x>>1) - 2);
			laser_y = ((rand255() & 3) + (DownRight.y>>1) - 2);
			return 2;
		}
	}

	return 0;
}


void draw_laser_lines (void)
{
  
  
  Vertex vTriangle[3];
  vTriangle[0].x = TopLeft.x+30;
  vTriangle[0].y = DownRight.y-1;
  vTriangle[0].z = 2;
  vTriangle[1].x = laser_x;
  vTriangle[1].y = laser_y;
  vTriangle[1].z = 2;
  vTriangle[2].x = TopLeft.x+40;
  vTriangle[2].y = DownRight.y-1;
  vTriangle[2].z = 2;
  
  if (game_state_flag & SOLID_SHIPS) {
    gba_setcolour(GFX_COL_DARK_RED);
    gba_triangle_clip(vTriangle); 
  } else {
    gba_setcolour(GFX_COL_WHITE);
    gfx_line_clip(vTriangle[0].x, vTriangle[0].y, ZMin,
    vTriangle[1].x, vTriangle[1].y, ZMin); 
    
    gfx_line_clip(vTriangle[2].x, vTriangle[2].y, ZMin,
    vTriangle[1].x, vTriangle[1].y, ZMin); 
    
  }
  vTriangle[0].x = DownRight.x-40;
  vTriangle[0].y = DownRight.y-1;
  vTriangle[0].z = 2;
  vTriangle[1].x = laser_x;
  vTriangle[1].y = laser_y;
  vTriangle[1].z = 2;
  vTriangle[2].x = DownRight.x-30;
  vTriangle[2].y = DownRight.y-1;
  vTriangle[2].z = 2;
  if (game_state_flag & SOLID_SHIPS) {
    gba_setcolour(GFX_COL_DARK_RED);
    gba_triangle_clip(vTriangle); 
  } else {
    gba_setcolour(GFX_COL_WHITE);
    gfx_line_clip(vTriangle[0].x, vTriangle[0].y, ZMin,
    vTriangle[1].x, vTriangle[1].y, ZMin); 
    
    gfx_line_clip(vTriangle[2].x, vTriangle[2].y, ZMin,
    vTriangle[1].x, vTriangle[1].y, ZMin); 
    
  }
  
  
  

}

void add_new_station (signed long int sx, signed long int sy, signed long int sz, Matrix rotmat)
{
	int station;

	station = (current_planet_data.techlevel >= 10) ? SHIP_DODEC : SHIP_CORIOLIS;
	universe[1].type = 0;
	add_new_ship (station, sx, sy, sz, rotmat, 0, -127);
	
	universe[1].flags |= FLG_HAS_ECM;
	
	  
}


void cool_laser (void)
{
	laser = 0;

	if (laser_temp > 0)
		laser_temp--;

	if (laser_counter > 0)
		laser_counter--;

	if (laser_counter > 0)
		laser_counter--;
}

void target_missile_toggle (void)
{
	if ((cmdr->missiles != 0) && (missile_target == MISSILE_UNARMED))
      missile_target = MISSILE_ARMED;
    else if (missile_target != MISSILE_UNARMED)
      missile_target = MISSILE_UNARMED;

}


void check_target (unsigned char un, Object *flip)
{
    Object *univ;
    char str[30];
	univ = &universe[un];
    
	if (in_target (univ->type, flip->location.x>>8, flip->location.y>>8, flip->location.z>>8))
	{
		if ((missile_target == MISSILE_ARMED) && (univ->type > 0))
		{
			missile_target = un;
			if (cmdr->equipment & ID_UNIT)
              sprintf (str, "Target %s", ELITE_SHIPS.pShipDataTable[univ->type].pShipData->cShipName);
            else
              strcpy (str, "Target Locked");

			info_message (str);
			PlaySoundFX(SND_BEEP);

		}

		if (laser)
		{

            PlaySoundFX(SND_HIT_ENEMY);
            enemy_hit = 1;
            
            
			if ((univ->type != SHIP_CORIOLIS) && (univ->type != SHIP_DODEC))
			{
				if ((univ->type == SHIP_CONSTRICTOR) || (univ->type == SHIP_COUGAR))
				{
					if (laser == (MILITARY_LASER & 127))
						univ->energy -= laser >>2;
				}
				else
				{
					univ->energy -= laser;
				}
			}

			if (univ->energy <= 0)
			{
				explode_object (un);

				if (univ->type == SHIP_ASTEROID)
				{
					if (laser == (MINING_LASER & 127))
					    launch_loot (un, SHIP_ROCK);
				}
				else
				{
					launch_loot (un, SHIP_ALLOY);
					launch_loot (un, SHIP_CARGO);
				}
			}

			make_angry (un);
		}
	}
}

unsigned char in_target (signed char type, signed long int x, signed long int y, signed long int z)
{
	signed long int size;

	if (z < 0)
		return 0;

	size = ELITE_SHIPS.pShipDataTable[type].pShipData->size;

    if (abs(x) > sqrt(size) || abs(y) > sqrt(size))
        return 0;

	return ((x*x + y*y) <= size);
}


void fire_missile (void)
{
	signed char newship;
	Object *ns;
	Matrix rotmat;

	if (missile_target < 0)
		return;

        set_unit_matrix(rotmat);

	rotmat[0].x = -1<<FIXEDPOINT; 
        rotmat[2].z =  1<<FIXEDPOINT;

	newship = add_new_ship (SHIP_MISSILE, 0, -28<<8, 14<<8, rotmat, 0, 0);

	if (newship == -1)
	{
		info_message ("Missile Jammed");
		return;
	}

	ns = &universe[newship];

	ns->velocity = flight_speed * 2;
	ns->flags = FLG_ANGRY;
	ns->target = missile_target;

	if (universe[missile_target].type > SHIP_ROCK)
		universe[missile_target].flags |= FLG_ANGRY;

	cmdr->missiles--;
	missile_target = MISSILE_UNARMED;

//	snd_play_sample (SND_MISSILE);
PlaySoundFX(SND_MISSILE);
}


// THE BIGGIE - all the AI stuff is in here...

void tactics (unsigned char un)
{
	signed short int type;
	signed short int energy;
	unsigned char maxeng;
	unsigned short int flags;
	Object *ship;
	VertexFixed nvecf;
	Vertex nvec;
	signed long int cnt2 = 0x39; //0.223;
	signed long int direction;
	unsigned char attacking;

	ship = &universe[un];
	type = ship->type;
	flags = ship->flags;
	gba_setcolour(GFX_COL_WHITE);


	if ((type == SHIP_PLANET) || (type == SHIP_SUN))
		return;

	if (flags & FLG_DEAD)
		return;

	if (flags & FLG_INACTIVE)
		return;

	if (type == SHIP_MISSILE)
	{
		if (flags & FLG_ANGRY)
			missile_tactics (un);
		return;
	}

	if (((un ^ mcount) & 7) != 0)
		return;

	if ((type == SHIP_CORIOLIS) || (type == SHIP_DODEC))
	{
 		if (flags & FLG_ANGRY)
		{
			if (rand255() <  240)
				return;

			if (ship_count[SHIP_VIPER] >= 4)
				return;

			launch_enemy (un, SHIP_VIPER, FLG_ANGRY | FLG_HAS_ECM, 113);
			return;
		}

		launch_shuttle ();
		return;
	}

	if (type == SHIP_HERMIT)
	{
		if (rand255() > 200) // || flags & FLG_ANGRY)
		{
		    // hermits launch a ship to defend their territory. 
		    // one of SIDEWINDER, MAMBA, KRAIT or ADDER. 
		    // I have changed this to include a bushmaster too
		    // was SHIP_SIDEWINDER + (rand255() & 3)
			launch_enemy (un, SHIP_BUSHMASTER + (rand255() & 4), FLG_ANGRY | FLG_HAS_ECM, 113);
			ship->flags |= FLG_INACTIVE;
		}

		return;
	}


	if (ship->energy < ELITE_SHIPS.pShipDataTable[type].pShipData->energy)
		ship->energy++;

	if ((type == SHIP_THARGLET) && (ship_count[SHIP_THARGOID] == 0))
	{
		ship->flags = 0;
		ship->velocity >>= 1;
		return;
	}

	if (flags & FLG_SLOW)
	{
		if (rand255() > 50)
			return;
	}

	if (flags & FLG_POLICE)
	{
		if (cmdr->legal_status >= 64)
		{
			flags |= FLG_ANGRY;
			ship->flags = flags;
		}
	}

	if ((flags & FLG_ANGRY) == 0)
	{
		if ((flags & FLG_FLY_TO_PLANET) || (flags & FLG_FLY_TO_STATION))
		{
			auto_pilot_ship (&universe[un]);
		}

		return;
	}


	/* If we get to here then the ship is angry so start attacking... */

	if (ship_count[SHIP_CORIOLIS] || ship_count[SHIP_DODEC])
	{
		if ((flags & FLG_BOLD) == 0)
			ship->bravery = 0;
	}


	if (type == SHIP_ANACONDA)
	{
		if (rand255() > 200)
		{
		  unsigned char tmp_rand = rand255();
		  unsigned char tmp_type = SHIP_SIDEWINDER;
		  if (tmp_rand > 100 && tmp_rand < 200) {
		    tmp_type = SHIP_WORM;
		  } else if (tmp_rand >= 200) {
		    tmp_type = SHIP_BUG;
		  }
		  
			launch_enemy (un, tmp_type, FLG_ANGRY | FLG_HAS_ECM, 113);
			return;
		}
	}


	if (rand255() >= 250)
	{
		ship->rotz = rand255() | 0x68;
		if (ship->rotz > 127)
			ship->rotz = -(ship->rotz & 127);
	}

	maxeng = ELITE_SHIPS.pShipDataTable[type].pShipData->energy;
	energy = ship->energy;

	if (energy < (maxeng >> 1))
	{
		if ((energy < (maxeng >> 3)) && (rand255() > 230) && (type != SHIP_THARGOID))
		{
			ship->flags &= ~FLG_ANGRY;
			ship->flags |= FLG_INACTIVE;
			launch_enemy (un, SHIP_ESCAPE_CAPSULE, 0, 126);
			return;
		}

		if ((ship->missiles != 0) && (ecm_active == 0) &&
			(ship->missiles >= (rand255() & 31)))
		{
			ship->missiles--;
			if (type == SHIP_THARGOID)
				launch_enemy (un, SHIP_THARGLET, FLG_ANGRY, ship->bravery);
			else
			{
				launch_enemy (un, SHIP_MISSILE, FLG_ANGRY, 126);
				info_message ("INCOMING MISSILE");
			}
			return;
		}
	}

	nvecf.x = ship->location.x>>8;
	nvecf.y = ship->location.y>>8;
	nvecf.z = ship->location.z>>8;
	UnitLengthFixed(&nvecf);

	nvec.x = nvecf.x;
	nvec.y = nvecf.y;
	nvec.z = nvecf.z;

	direction = vector_dot_product (&nvec, &ship->rotmat[2]);
	direction >>=FIXEDPOINT;

	if 	((ship->distance < 8192) && (direction <= -0x354) && //-0.833
		 (ELITE_SHIPS.pShipDataTable[type].pShipData->laser_strength != 0))
	{


		if (direction <= -0x3AB)//0.917
			ship->flags |= FLG_FIRING | FLG_HOSTILE;

		if (direction <= -0x3E3) //0.972
		{
			damage_ship (ELITE_SHIPS.pShipDataTable[type].pShipData->laser_strength, ship->location.z >= 0);
			ship->acceleration--;
			if (((ship->location.z >= 0) && (front_shield == 0)) ||
				((ship->location.z < 0) && (aft_shield == 0)))
				PlaySoundFX(SND_INCOMMING_FIRE_2);  // snd_play_sample (SND_INCOMMING_FIRE_2);
			else
			  PlaySoundFX(SND_INCOMMING_FIRE_1); // snd_play_sample (SND_INCOMMING_FIRE_1);
    }
		else
		{
			nvec.x = -nvec.x;
			nvec.y = -nvec.y;
			nvec.z = -nvec.z;
			direction = -direction;
			track_object (&universe[un], direction, nvec);
		}

//		if ((abs(ship->location.z) < 768) && (ship->bravery <= ((rand255() & 127) | 64)))
		if (abs(ship->location.z>>8) <= 0x300)
		{
			ship->rotx = rand255() & 0x87;
			if (ship->rotx > 127)
				ship->rotx = -(ship->rotx & 127);

			ship->acceleration = 3;
			return;
		}

		if (ship->distance < 8192)
			ship->acceleration = -1;
		else
			ship->acceleration = 3;

		return;
	}

	attacking = 0;

	if ((abs(ship->location.z>>8) >= 768) ||
		(abs(ship->location.x>>8) >= 512) ||
		(abs(ship->location.y>>8) >= 512))
	{

		if (ship->bravery > (rand255() & 127))
		{
			attacking = 1;
			nvec.x = -nvec.x;
			nvec.y = -nvec.y;
			nvec.z = -nvec.z;
			direction = -direction;
		}
	}

	track_object (&universe[un], direction, nvec);
  if ((attacking == 1) && (ship->distance < 2048)) {
		if (direction >= cnt2)
		{
			ship->acceleration = -1;
			return;
		}
    if (ship->velocity < 6)
			ship->acceleration = 3;
		else
			if (rand255() >= 200)
				ship->acceleration = -1;
		return;
	}
	if (direction <= -0xAB) //0.167
	{
		ship->acceleration = -1;
		return;
	}

	if (direction >= cnt2)
	{
		ship->acceleration = 3;
		return;
	}

	if (ship->velocity < 6)
		ship->acceleration = 3;
	else
		if (rand255() >= 200)
			ship->acceleration = -1;

}





void track_object (Object *ship, signed long int direction, Vertex nvec)
{
	signed long int dir;
	signed char rat;
	signed long int rat2;

	rat = 3;
	rat2 = 0x72; // 0.111

	dir = vector_dot_product (&nvec, &ship->rotmat[1]);
    dir >>= FIXEDPOINT;

    if (direction < -0x372) //0.861
	{
		ship->rotx = (dir < 0) ? 7 : -7;
		ship->rotz = 0;
		return;
	}
	ship->rotx = 0;

	if ((abs(dir) * 2) >= rat2)
	{
		ship->rotx = (dir < 0) ? rat : -rat;
	}

	if (abs(ship->rotz) < 16)
	{
		dir = vector_dot_product (&nvec, &ship->rotmat[0]);
        dir >>= FIXEDPOINT;
		ship->rotz = 0;

		if ((abs(dir) * 2) > rat2)
		{
			ship->rotz = (dir < 0) ? rat : -rat;

			if (ship->rotx < 0)
				ship->rotz = -ship->rotz;
		}
	}

}

void missile_tactics (unsigned char un)
{
	Object *missile;
	Object *target;
	VertexFixed vec;
	Vertex nvec;
	signed long int direction;
	signed long int cnt2 = 0xE4;//0.223

	missile = &universe[un];

	if (ecm_active)
	{
//		snd_play_sample (SND_EXPLODE);
    PlaySoundFX(SND_EXPLODE);
		missile->flags |= FLG_DEAD;
		return;
	}

	if (missile->target == 0)
	{
		if (missile->distance < 0x200)
		{
		  // accurate distance...
		  signed long accdis;

          // get a more accurate distance calculation...
          accdis = gba_sqrt( (missile->location.x>>8)*(missile->location.x>>8) +
                                    (missile->location.y>>8)*(missile->location.y>>8) +
                                    (missile->location.z>>8)*(missile->location.z>>8));
          if (accdis < 256) {
			missile->flags |= FLG_DEAD;
//			snd_play_sample (SND_EXPLODE);
      PlaySoundFX(SND_EXPLODE);
			damage_ship (250, missile->location.z >= 0);
			return;
          }
		}

		vec.x = missile->location.x;
		vec.y = missile->location.y;
		vec.z = missile->location.z;
	}
	else
	{
		target = &universe[missile->target];

		vec.x =(missile->location.x - target->location.x)>>8;
		vec.y =(missile->location.y - target->location.y)>>8;
		vec.z =(missile->location.z - target->location.z)>>8;

		if ((abs(vec.x) < 256) && (abs(vec.y) < 256) && (abs(vec.z) < 256))
		{
			missile->flags |= FLG_DEAD;

			if ((target->type != SHIP_CORIOLIS) && (target->type != SHIP_DODEC))
				explode_object (missile->target);
			else
			  PlaySoundFX(SND_EXPLODE);

			return;
		}

		if ((rand255() < 16) && (target->flags & FLG_HAS_ECM))
		{
			activate_ecm (0);
			return;
		}
	}

	UnitLengthFixed(&vec);
	nvec.x = vec.x;
	nvec.y = vec.y;
	nvec.z = vec.z;
	direction = vector_dot_product (&nvec, &missile->rotmat[2]);
	direction >>= FIXEDPOINT;
	nvec.x = -nvec.x;
	nvec.y = -nvec.y;
	nvec.z = -nvec.z;
	direction = -direction;

	track_object (missile, direction, nvec);

	if (direction <= -0x2A) //0.167
	{
		missile->acceleration = -2;
		return;
	}

	if (direction >= cnt2)
	{
		missile->acceleration = 3;
		return;
	}

	if (missile->velocity < 6)
		missile->acceleration = 3;
	else
		if (rand255() >= 200)
			missile->acceleration = -2;
	return;
}



void launch_enemy (unsigned char un, signed char type,  unsigned short int flags, unsigned short int bravery)
{
	signed char newship;
//	Object *ns;

	newship = add_new_ship (type, universe[un].location.x, universe[un].location.y,
							universe[un].location.z, universe[un].rotmat,
							universe[un].rotx, universe[un].rotz);

	if (newship == -1)
	{
		return;
	}

//	ns = &universe[newship];

	if ((universe[un].type == SHIP_CORIOLIS) || (universe[un].type == SHIP_DODEC))
	{
		universe[newship].velocity = 32;
		universe[newship].location.x += universe[newship].rotmat[2].x * 2;
		universe[newship].location.y += universe[newship].rotmat[2].y * 2;
		universe[newship].location.z += universe[newship].rotmat[2].z * 2;
	}

	universe[newship].flags |= flags;
	universe[newship].rotz /= 2;
	universe[newship].rotz *= 2;
	universe[newship].bravery = bravery;

	if ((type == SHIP_CARGO) || (type == SHIP_ALLOY) || (type == SHIP_ROCK))
	{
		universe[newship].rotz = ((rand255() * 2) & 255) - 128;
		universe[newship].rotx = ((rand255() * 2) & 255) - 128;
		universe[newship].velocity = rand255() & 15;
	} 
	
}

void launch_shuttle (void)
{
  unsigned char randval = rand255();
//  gba_debug_16_16(4,0,randval);
	if ((ship_count[SHIP_TRANSPORTER] != 0) ||
		(ship_count[SHIP_SHUTTLE] != 0) ||
		(ship_count[SHIP_SHUTTLEV] != 0) ||
		(randval < 253) || (game_state_flag & AUTO_PILOT))
		return;

	signed char type = rand255() & 1 ? SHIP_SHUTTLE : SHIP_TRANSPORTER;
	if (type == SHIP_SHUTTLE && (current_planet_data.techlevel >= 8) && (rand255() & 1))
	  type = SHIP_SHUTTLEV;
	launch_enemy (1, type, FLG_HAS_ECM | FLG_FLY_TO_PLANET, 113);
}

void launch_loot (unsigned char un, short int loot)
{
	unsigned char i,cnt;

	if (loot == SHIP_ROCK)
	{
		cnt = rand255() & 3;
	}
	else
	{
		cnt = rand255();
		if (cnt >= 128)
			return;

		cnt &= ELITE_SHIPS.pShipDataTable[universe[un].type].pShipData->max_loot;
		cnt &= 15;
	}

	for (i = 0; i < cnt; i++)
	{
		launch_enemy (un, loot, 0,0);
	}
}

void make_angry (unsigned char un)
{
	signed short int type;
	unsigned short int flags;

	type = universe[un].type;
	flags = universe[un].flags;

	if (flags & FLG_INACTIVE)
		return;

	if ((type == SHIP_CORIOLIS) || (type == SHIP_DODEC))
	{
	    if (!(universe[un].flags & FLG_ANGRY))
		  universe[un].flags |= FLG_ANGRY;
		return;
	}

	if (type > SHIP_ROCK)
	{
		universe[un].rotx = 4;
		universe[un].acceleration = 2;
		if (!(universe[un].flags & FLG_ANGRY))
		  universe[un].flags |= FLG_ANGRY;
	}
}

signed char create_other_ship (signed short type)
{
	Matrix rotmat;
	signed long int x,y,z;
	signed char newship;
	
        set_unit_matrix(rotmat);

	z = 12000;
	x = 1000 + (GetRand(65535) & 8191);
	y = 1000 + (GetRand(65535) & 8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;

	newship = add_new_ship (type, x<<8, y<<8, z<<8, rotmat, 0, 0);

	return newship;
}



void random_encounter (void)
{
  // don't add ships if we are in the safe zone, 
  // unless we are on the asteroid mission
	if ((ship_count[SHIP_CORIOLIS] != 0) || (ship_count[SHIP_DODEC] != 0)) {
	  if (cmdr->mission == 7 && (cmdr->galaxy_number == cmdr->mission3_galaxy_number)
          && find_planet_number(docked_planet)==cmdr->mission3_planet_number
	      && cmdr->mission_counter < MISSION3_LIMIT)
	    check_for_asteroid_mission();
		return;
	}
	
  // create a "rare" ship
	if (rand255() == 136)
	{
		if (((int)(universe[0].location.z>>8) & 0x3e) != 0)
			create_thargoid ();
		else
			create_cougar(); // blimey, that'll NEVER appear! ;)

		return;
	}
    
    
  if ((rand255() & 7) == 0)
	{
		create_trader();
		return;
	}

	check_for_asteroids();

	check_for_cops();
    
    // If a viper appears, don't create any pirates!
	if (ship_count[SHIP_VIPER] != 0)
		return;
    
    // the in battle flag is set when a lone hunter or a "pack"
    // of pirates is created. Stops you from getting overwhelmed.
	if (swat_flag & IN_BATTLE)
		return;
    
    
    // the thargoid plans mission - hound the player with thargs in normal space
	if ((cmdr->mission == 5) && (rand255() >= 200))
		create_thargoid ();

	check_for_others();
}

void create_thargoid (void)
{
	signed char newship;
	
	newship = create_other_ship (SHIP_THARGOID);
	if (newship != -1)
	{
		universe[newship].flags = FLG_ANGRY | FLG_HAS_ECM;
		universe[newship].bravery = 113;

		if (rand255() > 64)
			launch_enemy (newship, SHIP_THARGLET, FLG_ANGRY | FLG_HAS_ECM, 96);
	}	
}


void create_cougar (void)
{
	signed char newship;

	if (ship_count[SHIP_COUGAR] != 0)
		return;
	
	newship = create_other_ship (SHIP_COUGAR);
	if (newship != -1)
	{
		universe[newship].flags = FLG_HAS_ECM; // | FLG_CLOAKED;
		universe[newship].bravery = 121;
		universe[newship].velocity = 18;
	}	
}

// Create a passive ship. One of the following:
// SHIP_COBRA3, SHIP_PYTHON, SHIP_BOA, SHIP_ANACONDA
void create_trader (void)
{
	signed char newship;
	unsigned char rnd;
	signed short type;

	type = SHIP_COBRA3 + (rand255() & 3);

	newship = create_other_ship (type);
	
	if (newship != -1)
	{
		universe[newship].rotmat[2].z = -1<<FIXEDPOINT;
		universe[newship].rotz = rand255() & 7;
		
		rnd = rand255();
		universe[newship].velocity = (rnd & 31) | 16;
		universe[newship].bravery = rnd >>1;

		if (rnd & 1)
			universe[newship].flags |= FLG_HAS_ECM;

	}
}



// creates a single angry ship
// this can be one of: SHIP_COBRA3_LONE, SHIP_ASP2, SHIP_PYTHON_LONE, SHIP_FER_DE_LANCE
void create_lone_hunter (void)
{
	signed char newship;
	unsigned char rnd;
	signed short type;

	if ((cmdr->mission == 1) && (cmdr->galaxy_number == 1) &&
		(docked_planet.d == 144) && (docked_planet.b == 33) &&
		(ship_count[SHIP_CONSTRICTOR] == 0))
	{
		type = SHIP_CONSTRICTOR;
	}
	else
	{
		rnd = rand255();
		type = SHIP_COBRA3_LONE + (rnd & 3) + (rnd > 127);
	}
		
	newship = create_other_ship (type);

	if (newship != -1)
	{
		universe[newship].flags = FLG_ANGRY;
		if ((rand255() > 200) || (type == SHIP_CONSTRICTOR))
			universe[newship].flags |= FLG_HAS_ECM;
		
		universe[newship].bravery = ((rand255() * 2) | 64) & 127;
		swat_flag |= IN_BATTLE;
	}	
}

/* Check for a random asteroid encounter... */

void check_for_asteroids (void)
{
	signed char newship;
	signed short type;

	if ((rand255() >= 35) || (ship_count[SHIP_ASTEROID] >= 3))
		return;

	if (rand255() > 253)
		type = SHIP_HERMIT;
	else
		type = SHIP_ASTEROID;
		
	newship = create_other_ship (type);
	
	if (newship != -1)
	{
//		universe[newship].velocity = (rand255() & 31) | 16; 
		universe[newship].velocity = 8;
		universe[newship].rotz = rand255() > 127 ? -127 : 127; 
		universe[newship].rotx = 16; 
	}
}

/* If we've been a bad boy then send the cops after us... */

void check_for_cops (void)
{
  
	signed char newship;
	unsigned short int offense;

	offense = (unsigned short int) carrying_contraband() * 2;
	if (ship_count[SHIP_VIPER] == 0)
		offense |= cmdr->legal_status;

	if (rand255() >= offense)
		return;

	newship = create_other_ship (SHIP_VIPER);
	
	if (newship != -1)
	{
		universe[newship].flags = FLG_ANGRY;
		if (rand255() > 245)
			universe[newship].flags |= FLG_HAS_ECM;
		
		universe[newship].bravery = ((rand255() * 2) | 64) & 127;
	}
}


void check_for_others (void)
{
	signed long int x,y,z;
	signed char newship;
	Matrix rotmat;
	short gov;
	unsigned char rnd;
	signed short type;
	unsigned char i;

	gov = current_planet_data.government; 
	rnd = rand255();
    
    
    // if government is not anarchy, possibly return now
    // so anarchy planets nearly always have more ships coming.
    if ((gov != 0) && ((rnd >= 90) || ((rnd & 7) < gov)))
		return;	

	if (rand255() < 100)
	{
		create_lone_hunter();
		return;
	}	

	/* Pack hunters... */
	
        set_unit_matrix(rotmat);

	z = 12000;
	x = 1000 + GetRand(8191);
	y = 1000 + GetRand(8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;

	rnd = rand255() & 3;
	
	for (i = 0; i <= rnd; i++)
	{
		type = SHIP_SIDEWINDER + (rand255() & rand255() & 7);
		newship = add_new_ship (type, x<<8, y<<8, z<<8, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].flags = FLG_ANGRY;
			if (rand255() > 245)
				universe[newship].flags |= FLG_HAS_ECM;
		
			universe[newship].bravery = ((rand255() * 2) | 64) & 127;
			swat_flag |= IN_BATTLE;
		}
	}
	
}


void check_for_asteroid_mission(void) {
  signed char newship;
  unsigned char rnd;
  Matrix rotmat;
  signed long int x,y,z;
  unsigned char i;
  
	if ((rand255() >= 225) || (ship_count[SHIP_ASTEROID] >= 15))
		return;
  
  // create 1 or 2 or 3 asteroids
  
  if (rand255() < 100)
	{
		// create a single stroid
	  newship = create_other_ship (SHIP_ASTEROID);
  	if (newship != -1)
  	{
  		universe[newship].velocity = 8;
  		universe[newship].rotz = rand255() > 127 ? -127 : 127; 
  		universe[newship].rotx = 16; 
  	}
		return;
	}
  
  set_unit_matrix(rotmat);
    
  z = 8000;
	x = 1000 + GetRand(8191);
	y = 1000 + GetRand(8191);

	if (rand255() > 127)
		x = -x;
	if (rand255() > 127)
		y = -y;

	rnd = rand255() & 3;
	
  for (i = 0; i <= rnd; i++)
	{
		newship = add_new_ship ((rand255() > 50)?SHIP_ASTEROID:SHIP_BOULDER, x<<8, y<<8, z<<8, rotmat, 0, 0);
		if (newship != -1)
		{
			universe[newship].velocity = (rand255() & 16) | 8; 
  		universe[newship].rotz = rand255() > 127 ? -127 : 127; 
  		universe[newship].rotx = 16; 
		}
	}
	
}


void activate_ecm (unsigned char ours)
{
	if (ecm_active == 0)
	{
		ecm_active = 32;
		if (ours)
		  swat_flag |= ECM_OURS;
		else
		  swat_flag &= ~ECM_OURS;
		PlaySoundFX(SND_ECM);
//		snd_play_sample (SND_ECM);
	}
}


void time_ecm (void)
{
	if (ecm_active != 0)
	{
		ecm_active--;
		if (swat_flag & ECM_OURS)
			decrease_energy (-1);
	}
}

void abandon_ship (void)
{
	int i;

	cmdr->equipment &= ~ESCAPE_POD;
	cmdr->legal_status = 0;
	cmdr->fuel = myship.max_fuel;
	
	for (i = 0; i < NO_OF_STOCK_ITEMS; i++)
		cmdr->current_cargo[i] = 0;
	
	PlaySoundFX(SND_DOCK);					
	dock_player();
	current_screen = SCR_BREAK_PATTERN_ESC_POD;
}

