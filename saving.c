// saving handler
#include "saving.h"


void SaveByte(unsigned short nOffset, unsigned char cValue);
unsigned char LoadByte(unsigned short nOffset);


unsigned short GetLoadDataSize(char* pKey) {
  // 1st check the key is from our game...
  unsigned short i;
  for (i = 0; i < 3; i++) {
    if (LoadByte(i) != pKey[i]) {
      return 0;
    }
  }
  // is our game! how much ram to load the data?
  unsigned short nSize = LoadByte(3) + LoadByte(4)*256;
  // byte 4,5 are the data size
  
  if (nSize > (1<<15)) {
    // too much data..
    return 0;
  }
  return nSize+1;
}


void LoadData(unsigned char * pDataDest, char* pKey) {
  // load the data which has the given pKey
  // pDataDest pointer is already alloc'd
  
  // the save structure is:
  // byte 0, 1, 2 - Key, e.g. "CYC" for cyclone
  // byte 3, 4 - size of save data, not including key or these bytes (byte[3]+ byte[4]*256)
  // byte 5, n+5 - the save data
  
  unsigned short i;
  
  // set OK flag
  pDataDest[0] = 0; // 0 for fail, 1 for OK
  
  // check is our game - get data size..
  unsigned short nSize = GetLoadDataSize(pKey);
  if (nSize == 0) {
    // too much data..
    pDataDest[1] = 2;
    return;
  }
  // ok to here...
  
  for (i = 0; i < nSize; ++i) {
    pDataDest[i+1] = LoadByte(5+i);
  }
  pDataDest[0] = 1;
    
  // all done!
}

unsigned char LoadByte(unsigned short nOffset) {
  return *(unsigned char *)(SRAM + nOffset);
}


void SaveData(unsigned char * pDataSrc, unsigned short nDataLength, char* pKey) {
  // the save structure is:
  // byte 0, 1, 2 - 3 byte Key, e.g. "CYC" for cyclone
  // byte 3, n - the save data - size is stored in bytes 4,5 and 6
  
  unsigned short i; // looping var
  
  // 1st save the key from our game...
  for (i = 0; i < 3; i++) {
    SaveByte(i, pKey[i]);
  }
  
  
  // save the size of the data...
  unsigned char nSizeLo = nDataLength& 0xff;
  unsigned char nSizeHi = (nDataLength>>8) & 0xff;
  SaveByte(3, nSizeLo); SaveByte(4, nSizeHi);
  
  // save the actual data
  for (i = 0; i < nDataLength; i++) {
    SaveByte(5+i, pDataSrc[i]);
  }
  
  
}

void SaveByte(unsigned short nOffset, unsigned char cValue) {
  //Save the value at the given offset
  *(unsigned char *)(SRAM + nOffset) = cValue;
}



  

