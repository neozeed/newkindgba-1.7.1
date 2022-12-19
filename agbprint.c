#include "agbprint.h"
#include <stdio.h>
#include <stdarg.h>

//GBA - Visualboy Advance print routines

// compile thumb
void agbprint(char *s)
{
    
     asm volatile("mov r0, %0;"
                  "swi 0xff;"
                  : // no ouput
                  : "r" (s)
                  : "r0");
}    
// compile arm
//void agbprint(char *s)
//{
// asm volatile("mov r0, %0;"
//              "swi 0xff0000;"
//              : // no ouput
//              : "r" (s)
//              : "r0");
//}

void gbaprintf(char *fmt, ...)
{
  va_list argp;
  // allocate a buffer to print to...
  char str[200];
  
  va_start(argp, fmt);
  vsprintf(str, fmt, argp);
  va_end(argp);
  // print the line of text followed by a newline
  agbprint(str);
  agbprint("\n");
}
