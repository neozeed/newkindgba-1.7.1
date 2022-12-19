#ifndef SWAT_H
#define SWAT_H

#include "space.h"

#define MISSILE_UNARMED	-2
#define MISSILE_ARMED	-1

extern signed char missile_target;
extern unsigned long int swat_flag;
extern signed char ecm_active;
extern unsigned char enemy_hit;
extern unsigned char laser_x, laser_y;

#define ECM_OURS 1
#define IN_BATTLE 2


void arm_missile (void);
void unarm_missile (void);
void fire_missile (void);

void activate_ecm (unsigned char ours);
void time_ecm (void);
void reset_weapons (void);
void clear_universe (void);
void explode_object (unsigned char un);
void remove_ship (unsigned char un);

unsigned char fire_laser (void);
void cool_laser (void);
void draw_laser_lines (void);
void add_new_station (signed long int sx, signed long int sy, signed long int sz, Matrix rotmat);

void tactics (unsigned char un);

void target_missile_toggle (void);
void check_target (unsigned char un, Object *flip);
void fire_missile (void);

void random_encounter (void);

void abandon_ship (void);

void create_thargoid (void);
#endif

