// General calls used for saving data...

#define SRAM 0x0E000000

/*

  Saving/loading data is straight forward, just read and write to 0x0E000000 
  as if it were normal memory. 
  
  The pKey are pointers to char arrays - I use this so I know it's my game data in there.
  In the case of flash cards, I don't know how this stuff is handled, so if there is old
  save data in the mem location, I don't want to read that and get all confused.
  
  To do - implement packing/unpacking of the data.
  

*/

unsigned short GetLoadDataSize(char* pKey);
void LoadData(unsigned char * pDataDest, char* pKey);
void SaveData(unsigned char * pDataSrc, unsigned short nDataLength, char* pKey);







