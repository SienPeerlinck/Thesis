#include "crc.h"

/// From https://create.stephan-brumme.com/crc32/#tableless
/// compute CRC32 (byte algorithm) without lookup tables
unsigned int crc32(const void* data, unsigned int length, unsigned int previousCrc32)
{
  unsigned int crc = ~previousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
  const unsigned char* current = (const unsigned char*) data;
  while (length-- != 0)
  {
    unsigned char s = ((unsigned char)crc) ^ *current++;
    unsigned int low = (s ^ (s << 6)) & 0xFF;
    unsigned int a   = (low * ((1 << 23) + (1 << 14) + (1 << 2)));
    crc = (crc >> 8) ^
          (low * ((1 << 24) + (1 << 16) + (1 << 8))) ^
           a ^
          (a >> 1) ^
          (low * ((1 << 20) + (1 << 12)           )) ^
          (low << 19) ^
          (low << 17) ^
          (low >>  2);

  }
  //return ~crc; // same as crc ^ 0xFFFFFFFF
  return crc;
}
