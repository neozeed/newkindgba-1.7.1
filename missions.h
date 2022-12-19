#ifndef MISSIONS_H
#define MISSIONS_H

#define MISSION3_LIMIT 30

const char *mission_planet_desc (struct galaxy_seed planet);
const char *mission3_planet_desc (struct galaxy_seed planet);

char mission3_planet_name[16];


void check_mission_brief (void);

#endif

