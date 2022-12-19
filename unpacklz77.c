#include "UnpackLZ77.h"

void LZ77GetUnpackedSize(const unsigned char* inBuffer, unsigned short* nSize) {
  unsigned short lSize =  inBuffer[0x1] |
              (inBuffer[0x2]*256) |
              (inBuffer[0x3]*65536);
  lSize += 20; // just in case..  
  *nSize = lSize;
  
}


void LZ77UnCompWRAM(const unsigned char* inBuffer, unsigned char* outBuffer) {

// if someone ever figures this out then this is how to use the BIOS calls.
//  asm volatile(
//    "mov r0, %0 \n"
//    "mov r1, %1 \n"
//    "swi 0x110000 ": 
//    /* No output */ : 
//    "r" (inBuffer) , "r" (outBuffer) : 
//    "r0", "r1");
  
  // do "by hand" for now because it works!
  // initialise stuff...
  
  unsigned long nDecompressedPos     = 0; // start of the decompression array
  unsigned long nCompressedStreamPos = 1; // start of the compressed array - byte 0 is the compression type flag (I think)
  
  // find decompressed size...
  unsigned short nDSize = inBuffer[0x1] |
              (inBuffer[0x2]*256) |
              (inBuffer[0x3]*65536);//(unsigned long)(inBuffer[nCompressedStreamPos] + (inBuffer[nCompressedStreamPos+1]<<8) + (inBuffer[nCompressedStreamPos+2]<<16));
  nCompressedStreamPos = 4;

  unsigned char cByte1;
  unsigned char cByte2;
                  
  unsigned char cLength;
  unsigned char cOffset_Hi;
  unsigned char cOffset_Low;  

  unsigned short nOffset;
  signed long nTempPos;
  unsigned char nBit;
  unsigned char cKey;
  // decompress until we fill our byte array...
  while(nDecompressedPos < nDSize) {
    //"The coded data. This is divided up into sections consisting of an 8 
    // bit key followed by a corresponding eight items of varying size." - Cowbite spec
    cKey = inBuffer[nCompressedStreamPos];
    nCompressedStreamPos++;
    // "The upper bits in the key correspond to the items with lower addresses and vice versa" - Cowbite spec
    
    for (nBit = 0; nBit < 8; nBit++) {
      // was this a single byte, or a offset/length pair?
      if ((cKey & (1<<(7-nBit))) == 0) {
        // an uncompressed byte... add it to the output
          if (nDecompressedPos < nDSize)
		    outBuffer[nDecompressedPos] = inBuffer[nCompressedStreamPos];
		  else
		    return;
		  nDecompressedPos++;
		  nCompressedStreamPos++;
      } else {
        // Read the offset and length  
        /*
        "The top four bits being the number of bytes to output minus 3, the bottom 12 bits being 
        the offset behind the current window position from which to output" - Cowbite spec (reworded slightly)
        */
        cByte1 = inBuffer[nCompressedStreamPos];
        nCompressedStreamPos++;
        cByte2 = inBuffer[nCompressedStreamPos];
        nCompressedStreamPos++;

        cLength  = (((cByte1>>4) & 0xf) +3);
        cOffset_Hi  = (cByte1 & 0x0F);
        cOffset_Low  = cByte2;
        
        nOffset = (cOffset_Low + (cOffset_Hi * 256));
        
        nTempPos = (nDecompressedPos - nOffset-1);
        
        while (cLength > 0) {
          cLength--;
          // error check for nTempPos - tries occasionally to match data before the start of the buffer
          // error check for nDecompressedPos - towards the end of the compressed data, might not have enough bytes
          // then it may try overwriting past the end of the buffer as it picks random bytes from memory
          if (nDecompressedPos < nDSize) {
            if (nTempPos < 0)
              outBuffer[nDecompressedPos] = 0x20;
            else
              outBuffer[nDecompressedPos] = outBuffer[nTempPos];
              
          } else 
            return;
          
          nDecompressedPos++;
          nTempPos++;
        }
      }
    }
  }  
     
}
