#include "keypad.h"
volatile unsigned long int* KEYS = (volatile unsigned long int*)0x04000130;

//void ShowKeys(void);
unsigned long int thisKeys;
unsigned long int lastKeys;
//unsigned long int storeKeys[10]; // store the last 5 key values
unsigned long int repeated;


unsigned char KeyPressed(unsigned short int key) {
  if (!(thisKeys & key))
    return 1;
  return 0;
}

unsigned char KeyPressedNoBounce(unsigned short int key, unsigned long int r){
  if (!(thisKeys & key)) { // if the recent poll has the key pressed
    if ((lastKeys & key) || (!(lastKeys & key)&& r < repeated)) { // and it wasn't pressed last time..
      repeated = 0;
      return 1;
    }
    else if (!(lastKeys & key))
      repeated++;
  }
  return 0;
}
void InitKeys(void) {
  lastKeys = 0xFFFF;
  thisKeys = 0xFFFF;
}

void UpdateKeys(void){
  lastKeys = thisKeys;
  thisKeys = *KEYS;
}
