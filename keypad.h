/***********************\
* 	Keypad.h		*
*	by dovoto		*
*				*
\***********************/

#ifndef KEYPAD_H
#define KEYPAD_H
#define KEY_A 		1
#define KEY_B 		2
#define KEY_SELECT	4
#define KEY_START 	8
#define KEY_RIGHT 	16
#define KEY_LEFT 		32
#define KEY_UP 		64
#define KEY_DOWN 		128
#define KEY_R		256
#define KEY_L 		512
extern volatile unsigned long int * KEYS;

void InitKeys(void);
void UpdateKeys(void);
unsigned char KeyPressed(unsigned short int  key);
unsigned char KeyPressedNoBounce(unsigned short int  key, unsigned long int r);


#endif
