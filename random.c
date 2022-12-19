/*
  Random number routine as posted to the gbadev list 
  by John Seghers <johnse at spiritdancers dot org>
  
  Uses linear congruential, this formula is attributed to Ahrens & Dieter
  LCG(2^32, 663608941, 0, 1)
  
*/

#include "random.h"

unsigned long int m_nRandom = 14071977; // Start seed. My birthday.

void SeedRand(unsigned long int  seed) {
  m_nRandom = seed;
}
void ChurnRand(void) {
  m_nRandom *= 663608941; // churn the random number
}


unsigned long int  GetRand(unsigned long int  nMax) {
  unsigned long long nResult = nMax;
  nResult = nResult * m_nRandom;
  ChurnRand();
  return (unsigned long int)(nResult >> 32);
}

unsigned long int GetRandSeed(void) {
  return m_nRandom;
}
// return a random value up to 255, used to make porting easier
unsigned char rand255(void) {
  return GetRand(0xFFFF) & 255;

}
