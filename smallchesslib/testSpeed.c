/**
  For testing and optimizing evaluation speed.
*/

#include <stdio.h>
#include <time.h>
#include "smallchesslib.h"

#define DEPTH 3
#define EXTRA_DEPTH 4

int main(void)
{
  SCL_Board b;
  SCL_boardFromFEN(b,"r3kbn1/p1p2ppp/bpn1pq1r/3p4/8/NPQBPN2/P1PP1PPP/R1B1K2R w KQq - 0 1");

  SCL_printBoardSimple(b,putchar,255,SCL_PRINT_FORMAT_UTF8);

  printf("computing moves (depth: %d, extra: %d)\n",DEPTH,EXTRA_DEPTH);

  uint8_t s0, s1;
  char p;

  uint32_t t = clock();

  SCL_getAIMove(b,DEPTH,EXTRA_DEPTH,0,SCL_boardEvaluateStatic,SCL_randomSimple,0,0,0,&s0,&s1,&p);
 
  t = clock() - t;
 
  printf("done: %d %d\n",s0,s1);

  printf("it took %d ticks\n",t);

  return 0;
}
