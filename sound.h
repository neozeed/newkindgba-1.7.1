#ifndef SOUND_H
#define SOUND_H

#define SND_LAUNCH		18         
#define SND_CRASH		9            
#define SND_DOCK		10           
#define SND_GAMEOVER	13         
#define SND_PULSE		20           
#define SND_HIT_ENEMY	14         
#define SND_EXPLODE		12         
#define SND_ECM			11           
#define SND_MISSILE		19         
#define SND_HYPERSPACE	15       
#define SND_INCOMMING_FIRE_1	16 
#define SND_INCOMMING_FIRE_2	17 
#define SND_BEEP		6            
#define SND_BOOP		7            
#define SND_SELECT		21         
#define SND_CANCEL		8          

void ResetSoundRegs(void);
void Timer0UpdateSound(void);// __attribute__ ((section(".iwram"))); //the interrupt handle from crt0.s
void PlaySoundFX(unsigned char nSample);

void PlaySong(unsigned char nSong);
void StopSong(void);
// call this on every Timer 0 interupt


// 

#endif

