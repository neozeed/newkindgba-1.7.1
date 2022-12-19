
#ifndef SPACE_H
#define SPACE_H

#include "vector.h"

#define MAX_UNIV_OBJECTS	12

struct object {
  signed short int type;
  signed short rotx;
  signed short rotz;
  Matrix rotmat; // shorts
  VertexFixed location; // longs
  unsigned short int flags;
	signed short int energy;
	signed short int velocity;
	signed short int acceleration;
	signed char missiles;
	unsigned short int target;
	unsigned short int bravery;
	unsigned short int exp_delta; // used for explosions
	unsigned long int exp_seed;
	signed long int distance;
	unsigned char colour;
};

typedef struct object Object;

extern Object * universe;
extern unsigned short int * ship_count;

void dock_player (void);

void update_console (void);
void update_universe (int plot_starfield);
void update_altitude (void);

signed char add_new_ship (signed short int ship_type, 
								signed long int x, signed long int y, signed long int z, 
								Vertex *rotmat, 
								signed short int rotx, signed short int rotz);

void damage_ship (signed short int damage, signed short int front);

void launch_player (void);

void increase_flight_roll (void);
void decrease_flight_roll (void);
void increase_flight_climb (void);
void decrease_flight_climb (void);

void display_front_view(void);
void display_rear_view(void);
void display_left_view(void);
void display_right_view(void);

void engage_docking_computer (void);
void display_hyper_status (void);
void start_hyperspace (void);
void start_galactic_hyperspace (void);
void countdown_hyperspace (void);

void jump_warp (void);
void regenerate_shields (void);
void update_cabin_temp (void);

void docking_computer_selected(void);
void ecm_selected(void);
void escape_capsule_selected(void);
void energy_bomb_selected(void);

void initialise_universe(void);
void decrease_energy (signed short int amount);
unsigned char rotate_byte_left (unsigned char x);
#endif

