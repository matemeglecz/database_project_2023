#include <stdio.h>
#include <time.h>
#include "../smallchesslib.h"

SCL_Board startState = SCL_BOARD_START_STATE;

const char promotions[] = "qrkb";

int main(void)
{
  uint16_t seed = (uint16_t) time(0) + clock();
  SCL_randomBetterSeed(seed);

  SCL_Game game;
  SCL_gameInit(&game,startState);

  int moves = SCL_randomBetter() % 100;

  for (int i = 0; i < moves; ++i)
  {
    if (game.state != SCL_GAME_STATE_PLAYING)
    {
      SCL_gameUndoMove(&game);
      break;
    }

    uint8_t squareFrom = 0;
    uint8_t squareTo = 0;
    char p;

    if (i == 6 || i == 20) // randomly alter seed
      SCL_randomBetterSeed((uint16_t) time(0) + clock());
          
    SCL_getAIMove(
      game.board,
        SCL_randomBetter() % 3,
        SCL_randomBetter() % 4,
      0,
      SCL_boardEvaluateStatic,
      SCL_randomBetter,
      SCL_randomBetter() % 4,-1,-1,&squareFrom,&squareTo,&p);

    SCL_gameMakeMove(&game,squareFrom,squareTo,promotions[SCL_randomBetter() % 4]);
  }

  char string[256];

  SCL_boardToFEN(game.board,string);

  printf("%s\n",string);

  return 0;
}
