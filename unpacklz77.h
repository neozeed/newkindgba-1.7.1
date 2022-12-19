
/* 
  Implementation of LZSS decompression (despite the name ;))
  This is how the GBA does it, allegedly
  but I could never get the BIOS call to work with 
  any of the PD compression tools.
  
  The code is fully commented and fairly easy to follow I hope!
    
*/

void LZ77GetUnpackedSize(const unsigned char * inBuffer, unsigned short* nSize);
void LZ77UnCompWRAM(const unsigned char* inBuffer, unsigned char* outBuffer);

