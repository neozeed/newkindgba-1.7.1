
#ifndef RAND_H
#define RAND_H


void SeedRand(unsigned long int seed);
void ChurnRand(void);
unsigned long int GetRand(unsigned long int nMax);
unsigned char rand255(void);

unsigned long int GetRandSeed();

#endif

