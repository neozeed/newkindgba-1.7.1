#include "keypad.h"
#include "elite.h"
#include "sleep.h"
#include "interrupts.h"
#include "krawall.h"

#include "registers.h"
// added before first compilation succeeded, TM

extern unsigned char subseconds; 
extern unsigned char seconds;    
extern unsigned char minutes;    
extern unsigned short hours;     
void VblankIntr(void) {
    kramWorker();
      
    frames++;
    subseconds++;
    if (subseconds == 60) {
      seconds++;
      subseconds = 0;
    }
    if (seconds == 60) {
      minutes++;
      seconds = 0;
    }
    if (minutes == 60) {
      hours++;
      minutes = 0;
    }
    // tell the hardware which interupt took place
     REG_IF = INT_VBLANK;
    // let the bios know that vblank occurred
    (*(volatile u32*)0x03fffff8) = INT_VBLANK;  
}
// setup joypad interrupt function to call Sleep_Mode with a param of 0 (sleep off)
void JoypadIntr(void)
{
  if (isSleepMode == 1) {
    // shuts off sleep mode
    Sleep_Mode(0);
  }
  InitKeys();
}


