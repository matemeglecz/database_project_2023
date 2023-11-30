/**
  Script for converting the piece graphic to C data.

  CC0
*/

#include <stdio.h>
#include <stdint.h>

int main(void)
{
  FILE *f = fopen("pieces.ppm","rb");
  uint8_t b[256];
  fread(b,1,0x3A,f);
  int counter = 0;
  uint8_t byte = 0;

  while (fread(b,1,3,f) == 3)
  {
    byte = (byte << 1) | (b[0] != 0);

    counter++;

    if (counter % 8 == 0)
    {
      printf("0x%x,",byte);
   
      if ((counter % (8 * 12)) == 0)
        putchar('\n');

      byte = 0;
    }
  }
    
  fclose(f);

  return 0;
}
