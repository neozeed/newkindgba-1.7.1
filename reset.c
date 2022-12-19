#include "gba.h"
#include "reset.h"

#define ROM_BANKSWITCH (volatile u16 *)(0x096B592E)
#define WRITE_LOC_1 (volatile u16 *)(0x987654*2+0x8000000)
#define WRITE_LOC_2 (volatile u16 *)(0x012345*2+0x8000000)
#define WRITE_LOC_3 (volatile u16 *)(0x007654*2+0x8000000)
#define WRITE_LOC_4 (volatile u16 *)(0x765400*2+0x8000000)
#define WRITE_LOC_5 (volatile u16 *)(0x013450*2+0x8000000)

CODE_IN_EWRAM void reset_gba(void)
{
  unsigned int i;

  /* reset cart bankswitching */
  for(i=0;i<1;i++) *WRITE_LOC_1=0x5354;
  for(i=0;i<500;i++) *WRITE_LOC_2=0x1234;
  for(i=0;i<1;i++) *WRITE_LOC_2=0x5354;
  for(i=0;i<500;i++) *WRITE_LOC_2=0x5678;
  for(i=0;i<1;i++) *WRITE_LOC_1=0x5354;
  for(i=0;i<1;i++) *WRITE_LOC_2=0x5354;
  for(i=0;i<1;i++) *WRITE_LOC_4=0x5678;
  for(i=0;i<1;i++) *WRITE_LOC_5=0x1234;
  for(i=0;i<500;i++) *WRITE_LOC_2=0xabcd;
  for(i=0;i<1;i++) *WRITE_LOC_1=0x5354;
  *ROM_BANKSWITCH=0;
  /* reset GBA */
  *(u16 *)0x03007ffa = 0;  /* reset to ROM (= 0) rather than RAM (= 1) */
  asm volatile(
    "mov r0, #0xfc  \n"  /* clear everything other than RAM */
    "swi 0x01       \n"
    "swi 0x00       \n"
    ::: "r0", "r1", "r2", "r3");
}

