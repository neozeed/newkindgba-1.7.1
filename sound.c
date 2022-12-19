#include "sound.h"
//#include "sound_data.h"
#include "gbalib_asm.h"
#include "main.h"
#include "interrupts.h"

#include "registers.h"
// added before first compilation succeeded, TM

#include "krawall.h"
#include "mtypes.h"
#include "snd/modules.h"
#include "snd/samples.h"
//#include "nmodgcc.h"

//unsigned long int iNextSample; // index to the next section of sample to play
//unsigned long int SampleSize; // the size of the sample
//unsigned long int * pSample; // pointer to the current sample playing
//unsigned char SongPlaying;

//void TimerPlaySound(const unsigned long int * pS, unsigned long int nSize);

// to convert .mod to .o use:
//      bin2mod .rodata *.mod *.o
void PlaySong(unsigned char nSong) {
  if (nSong == 0) {
    krapPlay( &mod_theme, KRAP_MODE_LOOP, 0 );	// play module
  } else {
    krapPlay( &mod_danube, KRAP_MODE_LOOP, 0 );	// play module
  }
//  
//  NMOD_SetMasterVol(64,0);
//  NMOD_SetMasterVol(64,1);
//  NMOD_SetMasterVol(64,2);
//  NMOD_SetMasterVol(64,3);
//  unsigned long int regs = REG_IE;
//  if (nSong == 0) {
//    NMOD_Play( (u32)_binary_theme_mod_start );
////    krapPlay( &mod_theme, KRAP_MODE_LOOP, 0 );	// play module
//  } else {
//    NMOD_Play( (u32)_binary_danube_mod_start );
////    krapPlay( &mod_danube, KRAP_MODE_LOOP, 0 );	// play module
//  }
//  REG_IE |= regs;
//  SongPlaying = 1;
}

void StopSong(void) {
  krapStop();
//  // remember which regs we had irqs on before, as NMOD_Stop blitzes them
//  unsigned long int regs = REG_IE;
//  NMOD_Stop();
//  
//  // the following is needed so that the last note doesn't linger
//  // otherwise, starting another tune has the last note of the last tune 
//  // still playing for a while.
//  NMOD_pattern = 0;
//  NMOD_row = 0;
//  unsigned short i;
//  for (i = 0; i < BUFFER_SIZE; i++) {
//    NMOD_buffera[0][i] = 0;
//    NMOD_buffera[1][i] = 0;
//    NMOD_buffera[0][i] = 0;
//    NMOD_bufferb[1][i] = 0;
//  }
//  for (i = 0; i < 4; i++) {
//    NMOD_instrument[i] = 0;
//    NMOD_effect[i] = 0;
//    NMOD_period[i] = 0;
//    NMOD_volume[i] = 0;
//  }
//  // return our irqs, minus TIMER1
//  REG_IE |= regs;
//  REG_IE &= ~INT_TIMER1;

//  SongPlaying = 0;
  
}

// play a sound effect, the number is the index of the sample defined in the sound_data file
void PlaySoundFX(unsigned char nSample) {
  
  kramPlay( samples[ nSample ], 1, 0 );
  
//  if (nSample > SND_CANCEL || SongPlaying)
//   return;
//  REG_IME=0;				//turn off interrupts
//  TimerPlaySound(ELT_SoundTable[nSample]->pSample, ELT_SoundTable[nSample]->nLength);
//  REG_IME=1;				//enable interrupts
}

// an attempt to reset the sound regs, it doesn't work though :(
void ResetSoundRegs(void) {
  REG_IE = INT_VBLANK|INT_TIMER1;		//disable all interupt except VBLANK
  REG_SGFIFOA = 0;
  REG_DMA1DAD = 0;
  REG_DMA1SAD = 0;
  REG_DMA1CNT = 0;
  REG_DMA2DAD = 0;
  REG_DMA2SAD = 0;
  REG_DMA2CNT = 0;
  REG_SOUNDCNT = 0;
  REG_TM0CNT = 0;
  REG_TM0D = 0;
  REG_TM1CNT = 0;
  REG_TM1D = 0;

}


/*
--------------------------------------------------------------------------------
Direct Sound source code
By Uze, 2001
http://www.BeLogic.com
You may freely distribute, modify or use this source code without any limitations.

-Created in Visual Studio C++ 6.0
-Compiles with GCC's build "DevKitAdvance"
-Uses Jeff's crt0.s and lnkscript (Crtls v1.26) from http://devrs.com/gba

--------------------------------------------------------------------------------

*/

//void TimerPlaySound(const unsigned long int * pS, unsigned long int nSize) {
//	//Play a mono sound at 16khz in Interrupt mode Direct Sound
//	//uses timer 0 as sampling rate source
//	
//  pSample = pS;
//  SampleSize = nSize;
//  iNextSample = 0;
//	REG_SOUNDCNT_H=0x0B0F;  //DirectSound A + fifo reset + max volume to L and R
//	REG_SOUNDCNT_X=0x0080;  //turn sound chip on
//
//	REG_IE|=INT_TIMER0;		//enable timer 0 irq
//	
//	//set playback frequency
//	//note: using anything else thank clock multipliers to serve as sample frequencies
//	//		tends to generate distortion in the output. It has probably to do with timing and 
//	//		FIFO reloading.
//
//	REG_TM0CNT_L=0xffff; 
//	REG_TM0CNT_H=0x00C3;	//enable timer at CPU freq/1024 +irq =16386Khz sample rate
//	
//}
//
//void Timer0UpdateSound(void){
//
//	//Interrupt mode Direct Sound IRQ
//	//load FIFO each 4 samples with 4 bytes	
//	if(!(iNextSample&3))
//	  REG_SGFIF0A=pSample[iNextSample>>2];
//
//	iNextSample++;
//
//	if(iNextSample>SampleSize){
//		//sample finished!
//		REG_TM0CNT_H=0;	//disable timer 0
//	}
//
//}
//
