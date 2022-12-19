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
 * pilot.h
 */

#ifndef PILOT_H
#define PILOT_H

void fly_to_vector (Object *ship, VertexFixed vec);
void auto_pilot_ship (Object *ship);
void engage_auto_pilot (void);
void disengage_auto_pilot (void);

#endif
 
