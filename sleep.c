#include "gbalib_asm.h"
#include "elite.h"
#include "gba.h"
#include "sleep.h"
#include "keypad.h"

#include "registers.h"
// added before first compilation succeeded, TM

//GBA - "Sleep" Mode
//by Mike "BigRedPimp" Parks
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define REG_KEYCNT      *(volatile unsigned short *)0x04000132

unsigned char isSleepMode;
u16 intSoundCnt_X;
u16 intDispCnt;
u16 intKeyCnt;
u16 intIe;

void Sleep_Mode(int intSwitch)
{
  if (intSwitch == 1) // yes, sleep mode is needed
  {
    // store the original sound control register so we can return back to it
    intSoundCnt_X = REG_SOUNDCNT_X; 
    // shut off the sound hardware
    REG_SOUNDCNT_X = 0x00; 
    
    // store the original display control register so we can return back to it
    intDispCnt = REG_DISPCNT; 
    // shut off the screen
    REG_DISPCNT = 0x80; 
    
    // disable interrupts
    REG_IME = 0x00; 
    
     // store the original joypad interrupt control register so we can return back to it
    intKeyCnt = REG_KEYCNT;
    
    // set the key(s) you want to use to bring back the game back to normal
    // need to set the top 2 bits as well...
    REG_KEYCNT = 0xC000|KEY_L|KEY_R;
    
    intIe = REG_IE; // store the original interrupt control register so we can return back to it
    REG_IF = REG_IE = 0x1000; // enable interrupts based on the keypress
    
    REG_IME = 0x01; // enable interrupts
    
    isSleepMode = 1; // says we're in sleep mode
    
    // make a SWI call to the Stop function in BIOS
    // for thumb code, compiled with -mthumb, this should be asm("swi 3");
    asm("swi 3"); 
    
    // system now in sleep mode!
    
  } else if (intSwitch == 0) { // nope, not in sleep mode
    // set sound control to its original value
    REG_SOUNDCNT_X = intSoundCnt_X;
    // set display control to its original value
    REG_DISPCNT = intDispCnt;
    // set joypad interrupt control to its original value
    REG_KEYCNT = intKeyCnt;
    // disable interrupts
    REG_IME = 0x00;
    // set interrupt control to its original value
    REG_IF = REG_IE = intIe;
    // enable interrupts
    REG_IME = 0x01; 
    
    // says we're no longer sleeping
    isSleepMode = 0; 
    
    // reset temp values to 0
    intSoundCnt_X = 0; 
    intDispCnt = 0;    
    intKeyCnt = 0;     
  }
}
