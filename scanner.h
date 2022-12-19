// scanner.h 
// routines for drawing the scanner.

#ifndef SCANNER_H
#define SCANNER_H

#define RIGHT_SIDE_START 197



void display_speed(void);
void display_flight_climb(void);
void display_flight_roll(void);
void display_shields(void);
void display_altitude(void);
void display_energy(void);
void display_cabin_temp(void);
void display_laser_temp(void);
void display_fuel(void);
void display_missiles(void);


void update_scanner (void);
void update_compass (void);

#endif

