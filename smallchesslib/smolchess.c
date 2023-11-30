/**
  Simple chess engine using smallchesslib.

  author: Miloslav Ciz
  license: CC0 1.0 (public domain)
           found at https://creativecommons.org/publicdomain/zero/1.0/
           + additional waiver of all IP

  This work's goal is to never be encumbered by any exclusive intellectual
  property rights. The work is therefore provided under CC0 1.0 + additional
  WAIVER OF ALL INTELLECTUAL PROPERTY RIGHTS that waives the rest of
  intellectual property rights not already waived by CC0 1.0. The WAIVER OF ALL
  INTELLECTUAL PROPERTY RGHTS is as follows:

  Each contributor to this work agrees that they waive any exclusive rights,
  including but not limited to copyright, patents, trademark, trade dress,
  industrial design, plant varieties and trade secrets, to any and all ideas,
  concepts, processes, discoveries, improvements and inventions conceived,
  discovered, made, designed, researched or developed by the contributor either
  solely or jointly with others, which relate to this work or result from this
  work. Should any waiver of such right be judged legally invalid or
  ineffective under applicable law, the contributor hereby grants to each
  affected person a royalty-free, non transferable, non sublicensable, non
  exclusive, irrevocable and unconditional license to this right.  
*/

#include <stdio.h>
#include <time.h>

#define SCL_960_CASTLING 0 // setting to 1 compiles a 960 version of smolchess
#define XBOARD_DEBUG 0     // will create files with xboard communication
#define SCL_EVALUATION_FUNCTION SCL_boardEvaluateStatic

#define SCL_DEBUG_AI 0

#include "smallchesslib.h"

#if XBOARD_DEBUG
FILE *debugFile = 0;

char boardString[1024];
int boardStringPos = 0;

void putCharacterStr(char c)
{
  boardString[boardStringPos] = c;
  boardStringPos++;
}

void debugLogBoard(SCL_Board board)
{
  boardStringPos = 0;
  SCL_printBoardSimple(board,putCharacterStr,255,SCL_PRINT_FORMAT_UTF8);
  boardString[boardStringPos] = 0;
  fprintf(debugFile,"BOARD STATE:\n%s\n (%d %d)",boardString,
#if SCL_960_CASTLING
    board[SCL_BOARD_EXTRA_BYTE] & 0x07,
    board[SCL_BOARD_EXTRA_BYTE] >> 3
#else
    0,0
#endif
  );
}
#endif

uint8_t *pixels = 0;

void putImagePixel(uint8_t pixel, uint16_t number)
{
  uint16_t index = number * 3;

  pixel = pixel ? 255 : 0;

  pixels[index] = pixel;
  pixels[index + 1] = pixel;
  pixels[index + 2] = pixel;
}

void putCharacter(char c)
{
  putchar(c);
}

void xboardSend(const char *string)
{
#if XBOARD_DEBUG
  fprintf(debugFile,"SENDING: '%s'\n",string);
  fflush(debugFile);
#endif

  printf("%s",string); 
  fflush(stdout);
}

uint8_t stringsEqual(const char *s1, const char *s2, int max)
{
  for (int i = 0; i < max; ++i)
  {
    if (*s1 != *s2)
      return 0;
 
    if (*s1 == 0)
      return 1;

    s1++;
    s2++;
  }

  return 1;
}
  
uint8_t paramPlayerW = 0;
uint8_t paramPlayerB = 0;
uint8_t paramBoard = 1;
uint8_t paramAnalyze = 255; // depth of analysis
uint8_t paramMoves = 0;
uint8_t paramXboard = 0;
uint8_t paramInfo = 1;
uint8_t paramDraw = 0;
uint8_t paramFlipBoard = 0;
uint8_t paramHelp = 0;
uint8_t paramExit = 0;
uint16_t paramStep = 0;
char *paramFEN = NULL;
char *paramPGN = NULL;
uint16_t paramRandom = 0;
uint8_t paramBlind = 0;
int clockSeconds = -1;
SCL_Game game;
SCL_Board startState = SCL_BOARD_START_STATE;

int16_t random960PosNumber = -1;

int16_t makeAIMove(SCL_Board board, uint8_t *s0, uint8_t *s1, char *prom)
{
  uint8_t level = SCL_boardWhitesTurn(board) ? paramPlayerW : paramPlayerB;
  uint8_t depth = (level > 0) ? level : 1;
  uint8_t extraDepth = 3;
  uint8_t endgameDepth = 1;
  uint8_t randomness = 
    game.ply < 2 ? 1 : 0; /* in first moves increase randomness for different 
                             openings */
  uint8_t rs0, rs1;

  SCL_gameGetRepetiotionMove(&game,&rs0,&rs1);

  if (clockSeconds >= 0) // when using clock, choose AI params accordingly
  {
    if (clockSeconds <= 5)
    {
      depth = 1;
      extraDepth = 2;
      endgameDepth = 0;
    }
    else if (clockSeconds < 15)
    {
      depth = 2;
      extraDepth = 2;
    }
    else if (clockSeconds < 100)
    {
      depth = 2;
    }
    else if (clockSeconds < 5 * 60)
    {
      depth = 3;
    }
    else
    {
      depth = 3;
      extraDepth = 4;
    }
  }

  return SCL_getAIMove(board,depth,extraDepth,endgameDepth,SCL_boardEvaluateStatic,SCL_randomBetter,randomness,rs0,rs1,s0,s1,prom);
}

void printHelp(void)
{
#if SCL_960_CASTLING
  puts("smolchesss (960 random version)\n");
#else
  puts("smolchess\n");
#endif

  puts("Tiny public domain chess engine made with smallchesslib. Possible params:");
  puts("  -h   print help and exit");
  puts("  -x   start in xboard mode");
  puts("  -bN  board print format (1 - 4, 0 = don't draw board)");
  puts("  -PN  white player (0 = human, 1, 2, 3, ... AI)");
  puts("  -pN  black player");
  puts("  -aN  show AI evaluation and best move");
  puts("  -m   show possible moves and exit");
  puts("  -fS  load position from given FEN string");
  puts("  -gS  load position from given PGN string, combine with -s");
  puts("  -sN  specifies the ply number for -g");
  puts("  -i   show info during play (evaluation, PGN, ...)");
  puts("  -F   flip board");
  puts("  -e   exit immediately");
  puts("  -d   draw board to a ppm image file and end");
  puts("  -rN  set pseudorandom seed");
  puts("  -RN  load position by given 960 position number");
}

int main(int argc, char **argv)
{
  paramRandom = (uint16_t) time(0) + clock();

  for (int i = 0; i < argc; ++i)
  {
    char *a = argv[i];

    if (a[0] != '-')
      continue;

    switch (a[1])
    {
      case 'h': paramHelp = 1; break;             // print help
      case 'b': paramBoard = a[2] - '0'; break;   // board format
      case 'P': paramPlayerW = a[2] - '0'; break; // player 1 (white)
      case 'p': paramPlayerB = a[2] - '0'; break; // player 2 (black)
      case 'a': paramAnalyze = (a[2] == 0) ? 0 :
                  (a[2] - '0'); break;            // show AI evaluation and move
      case 'm': paramMoves = 1; break;            // show possible moves and end
      case 'x': paramXboard = 1; break;           // start as xboard engine
      case 'f': paramFEN = a + 2; break;          // set position (FEN)
      case 'e': paramExit = 1; break;             // exit 
      case 'g': paramPGN = a + 2; break;          // set position (PGN)
      case 's':                                   // set ply
        for (int j = 0; j < 4; ++j)
          if (a[j + 2] != 0)
          {
            paramStep *= 10;
            paramStep += a[j + 2] - '0';
          }
          else
            break;

        break;       

      case 'F': paramFlipBoard = 1; break;        // flip board (black down)
      case 'i': paramInfo = a[2] != '0'; break;   // show info
      case 'd': paramDraw = 1;                    // draw board to file and end
      case 'r':                                   // random seed
      case 'R':
        {
          uint16_t *n =
            a[1] == 'r' ? &paramRandom : &random960PosNumber;

          *n = 0;

          char *s = &(a[2]);

          while (*s != 0)
          {
            *n *= 10;
            *n += *s - '0';
            s++;
          }
        }

        break;

      default: puts("encountered unknown parameter"); break;
    }
  }

  if (paramHelp)
  {
    printHelp();
    return 0;
  }

  SCL_randomBetterSeed(paramRandom);

#if SCL_960_CASTLING
  if (random960PosNumber < 0)
    random960PosNumber = SCL_randomBetter();
#endif

  if (random960PosNumber >= 0)
    random960PosNumber %= 960;
 
  char string[256];

  if (paramFEN != NULL)
    SCL_boardFromFEN(startState,paramFEN);
  else if (paramPGN != NULL)
  {
    SCL_Record record;
    SCL_recordFromPGN(record,paramPGN);
    SCL_boardInit(startState);
    SCL_recordApply(record,startState,paramStep);
  }
#if SCL_960_CASTLING
  else
    SCL_boardInit960(startState,random960PosNumber);
#endif

  SCL_gameInit(&game,startState);

  SCL_SquareSet squareSet = SCL_SQUARE_SET_EMPTY;

  uint8_t move[] = {0, 0};

  if (!paramXboard)
  {
    // interactive CLI mode
  
    if (paramAnalyze != 255)
    {
      char p;

      paramPlayerW = paramAnalyze;
      paramPlayerB = paramAnalyze;

      int16_t evaluation = makeAIMove(game.board,&(move[0]),&(move[1]),&p);

      if (paramAnalyze == 0)
        evaluation = SCL_boardEvaluateStatic(game.board);

      char moveStr[5];
      moveStr[4] = 0;

      SCL_squareToString(move[0],moveStr);
      SCL_squareToString(move[1],moveStr + 2);

      printf("%lf (%d)\n",((double) evaluation) / ((double) SCL_VALUE_PAWN),evaluation);
      puts(moveStr);

      return 0;
    }
      
    if (paramMoves)
    {
      for (int i = 0; i < 64; ++i)
        if (game.board[i] != '.' && 
            SCL_pieceIsWhite(game.board[i]) == SCL_boardWhitesTurn(game.board))
        {
          SCL_SquareSet possibleMoves = SCL_SQUARE_SET_EMPTY;

          SCL_boardGetMoves(game.board,i,possibleMoves);

          SCL_SQUARE_SET_ITERATE_BEGIN(possibleMoves)
            SCL_moveToString(game.board,i,iteratedSquare,'q',string);
            printf("%s ",string);
          SCL_SQUARE_SET_ITERATE_END
        }

      return 0;
    }

    if (paramDraw)
    {
      uint8_t picture[3 * SCL_BOARD_PICTURE_WIDTH * SCL_BOARD_PICTURE_WIDTH];
      pixels = picture;

      SCL_drawBoard(game.board,putImagePixel,255,squareSet,paramFlipBoard);

      FILE *f = fopen("board.ppm","wb");

      char header[] = "P6 64 64 255\n";

      fwrite(header,sizeof(header) - 1,1,f);
      fwrite(picture,sizeof(picture),1,f);

      fclose(f);

      return 0;
    }

    char moveString[16];
    moveString[0] = 0;
    SCL_SquareSet moveHighlight = SCL_SQUARE_SET_EMPTY;

    while (1)
    {
      uint8_t moveType = 0; // 0: none, 1: player, 2: AI, 3: undo

      for (int i = 0; i < 40; ++i)
        putchar('\n');

      if (paramBoard)
        SCL_printBoard(game.board,putCharacter,moveHighlight,255,paramBoard,2,1,paramFlipBoard);

      putchar('\n');

      if (game.ply > 0)
      {
        printf(SCL_boardWhitesTurn(game.board) ? "black" : "white");
        printf(" played ");

        uint8_t s0, s1;
        char p;

        SCL_recordGetMove(game.record,game.ply - 1,&s0,&s1,&p);
        SCL_moveToString(game.board,s0,s1,p,moveString);
        printf("%s\n",moveString);
      }

      printf(SCL_boardWhitesTurn(game.board) ? "white" : "black");    
      printf(" to move\n");

      if (paramInfo)
      {
        putchar('\n');

        if (random960PosNumber >= 0)
          printf("960 random position number: %d\n",random960PosNumber);

        printf("ply number: %d\n",game.ply);
        SCL_boardToFEN(game.board,string);
        printf("FEN: %s\n",string);
        int16_t eval = SCL_boardEvaluateStatic(game.board);
        printf("board static evaluation: %lf (%d)\n",((double) eval) / ((double) SCL_VALUE_PAWN),eval);
        printf("board hash: %u\n",SCL_boardHash32(game.board));
        printf("phase: ");

        switch (SCL_boardEstimatePhase(game.board))
        {
          case SCL_PHASE_OPENING: puts("opening"); break;
          case SCL_PHASE_ENDGAME: puts("endgame"); break;
          default: puts("midgame"); break;
        }

        printf("en passant: %d\n",((game.board[SCL_BOARD_ENPASSANT_CASTLE_BYTE] & 0x0f) + 1) % 16);
        printf("50 move rule count: %d\n",game.board[SCL_BOARD_MOVE_COUNT_BYTE]);

        if (paramFEN == NULL && paramPGN == NULL)
        {
          printf("PGN: ");
          SCL_printPGN(game.record,putCharacter,startState);
          putchar('\n');
        }
      } 

      if (game.state != SCL_GAME_STATE_PLAYING || paramExit)
        break;
 
      uint8_t squareFrom = 0;
      uint8_t squareTo = 0;
      char movePromote = 'q';

      if (
        (SCL_boardWhitesTurn(game.board) && paramPlayerW == 0) || 
        (!SCL_boardWhitesTurn(game.board) && paramPlayerB == 0))
      {
        printf("\nmove: ");
        scanf("%s",string);

        if (stringsEqual(string,"undo",5))
          moveType = 3;
        else if (stringsEqual(string,"quit",5))
          break;
        else
        {
          squareFrom = SCL_stringToSquare(string);
          squareTo = SCL_stringToSquare(string + 2);

          uint8_t r = SCL_stringToMove(string,&squareFrom,&squareTo,&movePromote);

          if (r)
          {
            if ((game.board[squareFrom] != '.') &&
              (SCL_pieceIsWhite(game.board[squareFrom]) == SCL_boardWhitesTurn(game.board)))
              {
                SCL_boardGetMoves(game.board,squareFrom,squareSet);

                if (SCL_squareSetContains(squareSet,squareTo))
                {
                  moveType = 1;
                }
              }
          }
        }
      }
      else
      {
        makeAIMove(game.board,&squareFrom,&squareTo,&movePromote);
        moveType = 2;
      }

      if (moveType == 1 || moveType == 2)
      {
        SCL_moveToString(game.board,squareFrom,squareTo,movePromote,moveString);

        SCL_gameMakeMove(&game,squareFrom,squareTo,movePromote);

        SCL_squareSetClear(moveHighlight);
        SCL_squareSetAdd(moveHighlight,squareFrom);
        SCL_squareSetAdd(moveHighlight,squareTo);
      } 
      else if (moveType == 3)
      {
        if (paramPlayerW != 0 || paramPlayerB != 0)
          SCL_gameUndoMove(&game);

        SCL_gameUndoMove(&game);
        SCL_squareSetClear(moveHighlight);
      }
    }

    putchar('\n');

    switch (game.state)
    {
      case SCL_GAME_STATE_WHITE_WIN:
        puts("white wins");
        break;

      case SCL_GAME_STATE_BLACK_WIN:
        puts("black wins");
        break;

      case SCL_GAME_STATE_DRAW_STALEMATE:
        puts("draw (stalemate)");
        break;

      case SCL_GAME_STATE_DRAW_REPETITION:
        puts("draw (repeated position)");
        break;

      case SCL_GAME_STATE_DRAW_DEAD:
        puts("draw (dead position)");
        break;

      case SCL_GAME_STATE_DRAW:
        puts("draw");
        break;

      case SCL_GAME_STATE_DRAW_50:
        puts("draw (50 move rule)");
        break;

      default: 
        puts("game over"); 
        break;
    }
  }
  else
  {
    // xboard mode
    uint8_t started = 0;
    uint8_t playingWhite = 0;
    uint8_t editingWhite = 1;

    paramPlayerW = 3;
    paramPlayerB = paramPlayerW;

    setbuf(stdout,NULL);

#if XBOARD_DEBUG
    uint8_t randomLetter;

    FILE *randFile = fopen("/dev/random","rb");            
    fread(&randomLetter,1,1,randFile);
    fclose(randFile);
    char debugFileName[] = "/home/tastyfish/git/smallchesslib/xboard_debug_x.txt";
    debugFileName[47] = randomLetter % 26 + 'A';
    debugFile = fopen(debugFileName,"w");

    fprintf(debugFile,"MY RANDOM SEED IS: '%d'\n",paramRandom);
#endif

    SCL_boardInit(game.board);

    while (1)
    {
      char p;

      gets(string);

#if XBOARD_DEBUG
      fprintf(debugFile,"RECEIVED: '%s'\n",string);
#endif

      if (stringsEqual(string,"new",3))
      {
      }
      else if (stringsEqual(string,"go",2))
      {
        if (!started)
          playingWhite = SCL_boardWhitesTurn(game.board);
          
        started = 1;
      }
      else if (stringsEqual(string,"xboard",6))
      {
      }
      else if (stringsEqual(string,"c",1))
      {
        editingWhite = !editingWhite;
      }
      else if (stringsEqual(string,"#",1))
      {
        for (int i = 0; i < 64; ++i)
          game.board[i] = '.';
      }
      else if ((string[0] == 'P' || string[0] == 'R' || string[0] == 'N' ||
        string[0] == 'B' || string[0] == 'Q' || string[0] == 'K') &&
        string[1] != 0 && string[2] != 0 && string[3] == 0)
      {
        // place piece
        game.board[(string[2] - '1') * 8 + string[1] - 'a'] =
          SCL_pieceToColor(string[0],editingWhite);
      }
      else if (stringsEqual(string,"sd",2)) // search depth
      {
        paramPlayerW = string[3] - '0';

        if (paramPlayerW > 5) 
          paramPlayerW = 5;

        paramPlayerB = paramPlayerW;

#if XBOARD_DEBUG
        fprintf(debugFile,"SET SEARCH DEPTH TO %d\n",paramPlayerW);
#endif
      }
      else if (stringsEqual(string,"quit",4))
      {
        break;
      }
      else if (stringsEqual(string,"protover",8))
      {
        xboardSend("feature done=0\n");
        xboardSend("feature time=1\n");
        xboardSend("feature ping=1\n");
        xboardSend("feature setboard=1\n");
        xboardSend("feature sigint=0\n");
        xboardSend("feature sigterm=0\n");
        xboardSend("feature reuse=0\n");
        xboardSend("feature colors=0\n");
#if SCL_960_CASTLING
        xboardSend("feature variants=fischerandom\n");
#endif
        xboardSend("feature done=1\n");
      }
      else if (stringsEqual(string,"setboard",8))
      {
        SCL_boardFromFEN(game.board,string + 9);
      }
      else if (stringsEqual(string,"time",4))
      {
        clockSeconds = 0;
        const char *s = string + 5;

        while (*s != 0)
        {
          clockSeconds = clockSeconds * 10 + *s - '0';
          s++;
        }

        clockSeconds /= 100;
      }
      else if (stringsEqual(string,"ping",4))
      {
        char reply[64] = "pong ";

        const char *s = string + 5;
        char *s2 = reply + 5;  

        while (1)
        {
          *s2 = *s;

          if (*s == 0)
            break;

          s2++;
          s++;
        }

        xboardSend(reply);
        xboardSend("\n");
      }
      else if (stringsEqual(string,"O-O",3) || stringsEqual(string,"O-O-O",5))
      {
        uint8_t longCastling = string[3] != 0;
        uint8_t startPos = SCL_boardWhitesTurn(game.board) ? 0 : 56;
        uint8_t kingSquare = 0;

        for (int i = startPos; i < startPos + 8; ++i)
          if (game.board[i] == 'k' || game.board[i] == 'K')
          {
            kingSquare = i;
            break;
          }

        SCL_gameMakeMove(&game,kingSquare,startPos + (longCastling ? 
          (game.board[SCL_BOARD_EXTRA_BYTE] & 0x07) :
          (game.board[SCL_BOARD_EXTRA_BYTE] >> 3)),'q'); 
      }
      {
        uint8_t r = SCL_stringToMove(string,&(move[0]),&(move[1]),&p);

        if (r)
        {
          SCL_gameMakeMove(&game,move[0],move[1],p); 

          if (!started)
            playingWhite = SCL_boardWhitesTurn(game.board);

          started = 1;
        }
      }

      if (started && (playingWhite == SCL_boardWhitesTurn(game.board)))
      {
        makeAIMove(game.board,&(move[0]),&(move[1]),&p);

        SCL_moveToString(game.board,move[0],move[1],p,string);

        SCL_gameMakeMove(&game,move[0],move[1],p);

        xboardSend("move ");

#if SCL_960_CASTLING
        if ((game.board[move[0]] == 'K' && game.board[move[1]] == 'R') ||
          (game.board[move[0]] == 'k' && game.board[move[1]] == 'r'))
        {
          if (move[1] == (game.board[SCL_BOARD_EXTRA_BYTE] >> 3) ||
              move[1] == 56 + (game.board[SCL_BOARD_EXTRA_BYTE] >> 3))
            xboardSend("O-O");
          else
            xboardSend("O-O-O");
        }
        else
#endif
          xboardSend(string);

        xboardSend("\n");
      }

#if XBOARD_DEBUG
      SCL_boardToFEN(game.board,string);
      debugLogBoard(game.board);
      fprintf(debugFile,"FEN: '%s'\n",string);
#endif
    }
#if XBOARD_DEBUG
    fclose(debugFile);
#endif
  }

  return 0;
}
