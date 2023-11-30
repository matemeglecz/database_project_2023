/**
  Tests for smallchesslib. These are basic tests that should be run before
  every commit, just to catch major regressions.

  by drummyfish, released under CC0 1.0
*/

void putCharacter(char c)
{
  putchar(c);
}

char str[4096];

void putCharStr(char c)
{
  char *s = str;

  while (*s != 0)
    s++;

  *s = c;
  *(s + 1) = 0;
}

#include <stdio.h>
#include "smallchesslib.h"

uint8_t test(const char *str, uint8_t cond)
{
  printf("- testing %s: ",str);

  if (cond)
  {
    puts("OK");
    return 1;
  }

  puts("ERROR");
  return 0;
}

int strEquals(const char *s1, const char *s2)
{
  while (1)
  {
    if (*s1 != *s2)
      return 0;

    if (*s1 == 0 || *s2 == 0)
      break;

    s1++;
    s2++;
  }

  return 1;
}

int main(void)
{
  #define assert(str,cond) if (!test(str,cond)) return 1;
    
  SCL_SquareSet emptySquareSet = SCL_SQUARE_SET_EMPTY;

  {
    SCL_Board board = SCL_BOARD_START_STATE;

    assert("square color",SCL_squareIsWhite(SCL_stringToSquare("H1")))
    assert("square color",!SCL_squareIsWhite(SCL_stringToSquare("d4")))

    assert("piece color",SCL_pieceIsWhite('K'))
    assert("piece color",!SCL_pieceIsWhite('p'))

    assert("piece to color",SCL_pieceToColor('q',1) == 'Q')
    assert("piece to color",SCL_pieceToColor('R',1) == 'R')
    assert("piece to color",SCL_pieceToColor('p',0) == 'p')
    assert("piece to color",SCL_pieceToColor('K',0) == 'k')

    assert("square number", SCL_SQUARE('d',4) == SCL_stringToSquare("D4"))
    assert("square number", SCL_SQUARE('h',8) == SCL_stringToSquare("h8"))

    uint8_t s0, s1, r;
    char p;

    r = SCL_stringToMove("e2e4",&s0,&s1,&p);
    assert("string to move",s0 == SCL_S('e',2) && s1 == SCL_S('e',4) && r);

    r = SCL_stringToMove("H7G8q ",&s0,&s1,&p);
    assert("string to move",s0 == SCL_S('h',7) && s1 == SCL_S('g',8) && r && p == 'q');

    r = SCL_stringToMove("ie3!",&s0,&s1,&p);
    assert("illegal string to move",!r);

    puts("testing not crashing");
    r = SCL_stringToMove("aq",&s0,&s1,&p);

    SCL_boardInit960(board,950);
    assert("960",SCL_boardHash32(board) == 866912113);

    SCL_boardInit960(board,300);
    assert("960",SCL_boardHash32(board) == 3028682732);
  }

  {
    SCL_SquareSet s = SCL_SQUARE_SET_EMPTY;

    assert("empty square set",
      !SCL_squareSetContains(s,0) &&
      !SCL_squareSetContains(s,63) &&
      !SCL_squareSetContains(s,SCL_stringToSquare("c4")))

    SCL_squareSetAdd(s,SCL_stringToSquare("c4"));
 
    assert("square set contains",SCL_squareSetContains(s,SCL_stringToSquare("c4")))
  }

  {
    SCL_Board board = SCL_BOARD_START_STATE;

    puts("placing pieces");
    SCL_boardSetPosition(board,
      "r......."
      "p....p.p"
      ".p..pnpb"
      "n.p.P..."
      "B....qbP"
      "PkNQ...."
      ".PP..rPR"
      "R...KBN.",255,0,0);

    SCL_printBoard(board,putCharacter,emptySquareSet,255,SCL_PRINT_FORMAT_UTF8,4,1,0);

    assert("not check",!SCL_boardCheck(board,1));

    assert("check",SCL_boardCheck(board,0));

    assert("square not attacked",!SCL_boardSquareAttacked(board,SCL_stringToSquare("A8"),0));
    assert("square not attacked",!SCL_boardSquareAttacked(board,SCL_stringToSquare("H4"),1));
    assert("square attacked",SCL_boardSquareAttacked(board,SCL_stringToSquare("H8"),1));
    assert("square attacked",SCL_boardSquareAttacked(board,SCL_stringToSquare("G1"),0));

    puts("placing pieces");
    SCL_boardSetPosition(board,
      "R...K..R"
      ".p....PP"
      ".n......"
      "b...Q..."
      "........"
      "..P...p."
      "........"
      "r...k..r",255,0,1);

    SCL_printBoard(board,putCharacter,emptySquareSet,255,SCL_PRINT_FORMAT_UTF8,4,1,0);

    assert("check position",SCL_boardGetPosition(board) == SCL_POSITION_CHECK);

    SCL_SquareSet set;

    SCL_boardGetPseudoMoves(board,4,1,set);
    assert("king moves (castling)",SCL_squareSetSize(set) == 7);
 
    SCL_boardGetPseudoMoves(board,60,1,set);
    assert("king moves (castling)",SCL_squareSetSize(set) == 5);

    SCL_boardMakeMove(board,9,1,'q');

    assert("promotion",board[9] == '.' && board[1] == 'q');

    SCL_boardSetPosition(board,
      "........"
      "........"
      "........"
      "PpP....."
      ".....Pp."
      "........"
      "........"
      "........",255,0,0);

    board[SCL_BOARD_ENPASSANT_CASTLE_BYTE] = (char) 0xf6;

    SCL_boardGetPseudoMoves(board,37,1,set);
    assert("en-passant",SCL_squareSetSize(set) == 2);
    
    SCL_boardGetPseudoMoves(board,25,1,set);
    assert("en-passant",SCL_squareSetSize(set) == 1);
  }

  {
    SCL_Board board = SCL_BOARD_START_STATE;

    uint32_t hash = SCL_boardHash32(board);

    SCL_boardFromFEN(board,"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert("board from FEN (start position)",hash == SCL_boardHash32(board));

    SCL_boardFromFEN(board,"1nbqkb1r/pp5p/2p3pn/1r3p2/N2PPQ2/1Q5N/PP2BPPP/R1B2RK1 b k - 0 1");
    assert("board from FEN",SCL_boardHash32(board) == 2308982684);    
    
    assert("board from FEN (bad FEN)",SCL_boardFromFEN(board,"1nbqkb1r/pp5p/2p3pn/1r3p2/N2PPQ2/1Q5N/PP2BPPP/R1B2RK1 b k - 0") == 0);
    assert("board from FEN (bad FEN)",SCL_boardFromFEN(board,"1nb ass LLsasa LL221") == 0);
  }

  {
    SCL_Record r;

    SCL_recordInit(r);
    assert("record empty",SCL_recordLength(r) == 0);
    SCL_recordAdd(r,SCL_SQUARE('e',2),SCL_SQUARE('e',4),'q',SCL_RECORD_CONT);
    assert("record length = 1",SCL_recordLength(r) == 1);
    SCL_recordRemoveLast(r);
    SCL_recordRemoveLast(r);
    assert("record empty",SCL_recordLength(r) == 0);
    SCL_recordAdd(r,SCL_SQUARE('f',2),SCL_SQUARE('f',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',7),SCL_SQUARE('f',5),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',1),SCL_SQUARE('f',3),'q',SCL_RECORD_CONT);
    assert("record length = 3",SCL_recordLength(r) == 3);

    uint8_t s0, s1, e;
    char p;

    e = SCL_recordGetMove(r,1,&s0,&s1,&p);
    assert("record check move 1",
      (s0 == SCL_stringToSquare("f7")) && 
      (s1 == SCL_stringToSquare("f5")) &&
      (p == 'r') && (e == SCL_RECORD_CONT));

    e = SCL_recordGetMove(r,2,&s0,&s1,&p);
    assert("record check move 2",
      (s0 == SCL_stringToSquare("g1")) && 
      (s1 == SCL_stringToSquare("f3")) &&
      (p == 'q') && (e == SCL_RECORD_END));
    
    SCL_recordAdd(r,SCL_SQUARE('b',8),SCL_SQUARE('b',6),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',2),SCL_SQUARE('g',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',5),SCL_SQUARE('g',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',1),SCL_SQUARE('h',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('e',7),SCL_SQUARE('e',5),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('e',1),SCL_SQUARE('g',1),'q',SCL_RECORD_CONT);

    SCL_Board board = SCL_BOARD_START_STATE;
    SCL_recordApply(r,board,100);

    SCL_Board board2;
    
    SCL_boardSetPosition(board2,
      "RNBQ.RK."
      "PPPPP..P"
      ".....N.B"
      ".....Pp."
      "....p..."
      ".n......"
      "pppp..pp"
      "r.bqkbnr",207,1,9);

    SCL_SquareSet s = SCL_SQUARE_SET_EMPTY;

    assert("replaying record",!SCL_boardsDiffer(board,board2));
    SCL_printBoard(board,putCharacter,s,255,SCL_PRINT_FORMAT_UTF8,4,1,0);

    assert("normal position",SCL_boardGetPosition(board) == SCL_POSITION_NORMAL);
  }

  {
    SCL_Record r;

    SCL_recordInit(r);

    SCL_recordAdd(r,SCL_SQUARE('h',2),SCL_SQUARE('h',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',7),SCL_SQUARE('g',5),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('h',4),SCL_SQUARE('g',5),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',8),SCL_SQUARE('f',6),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',1),SCL_SQUARE('f',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',8),SCL_SQUARE('g',7),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('e',2),SCL_SQUARE('e',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('e',8),SCL_SQUARE('g',8),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('b',1),SCL_SQUARE('c',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',7),SCL_SQUARE('c',5),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',2),SCL_SQUARE('d',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',5),SCL_SQUARE('c',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('b',2),SCL_SQUARE('b',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',4),SCL_SQUARE('b',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',1),SCL_SQUARE('a',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',8),SCL_SQUARE('c',7),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',1),SCL_SQUARE('d',2),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('b',8),SCL_SQUARE('c',6),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('e',1),SCL_SQUARE('c',1),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',6),SCL_SQUARE('d',5),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',2),SCL_SQUARE('b',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',6),SCL_SQUARE('b',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',5),SCL_SQUARE('g',6),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',7),SCL_SQUARE('c',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',2),SCL_SQUARE('c',2),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',7),SCL_SQUARE('d',6),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',6),SCL_SQUARE('h',7),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',8),SCL_SQUARE('h',8),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('c',2),SCL_SQUARE('c',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',7),SCL_SQUARE('c',3),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('a',3),SCL_SQUARE('b',4),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('h',8),SCL_SQUARE('g',7),'q',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('h',7),SCL_SQUARE('h',8),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('d',5),SCL_SQUARE('b',4),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',1),SCL_SQUARE('e',2),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('g',7),SCL_SQUARE('f',6),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('h',1),SCL_SQUARE('f',1),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',6),SCL_SQUARE('g',7),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('f',1),SCL_SQUARE('h',1),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('b',4),SCL_SQUARE('a',6),'r',SCL_RECORD_CONT);
    SCL_recordAdd(r,SCL_SQUARE('h',1),SCL_SQUARE('h',4),'r',SCL_RECORD_CONT);

    str[0] = 0;

    SCL_printPGN(r,putCharStr,0);

    assert("to PGN",strEquals(str,
      "1. h4 g5 2. hxg5 Nf6 3. Nf3 Bg7 4. e3 O-O 5. Nc3 c5 6. d3 c4 7. b4 cxb3 8. Ba3 Qc7 9. Qd2 Nc6 10. O-O-O Nd5 11. cxb3 Ncb4 12. g6 Qxc3+ 13. Qc2 d6 14. gxh7+ Kh8 15. Qxc3 Bxc3 16. Bxb4 Kg7 17. h8=R Nxb4 18. Be2 Kf6 19. Rhf1 Kg7 20. Rfh1 Na6 21. R1h4*"));

    SCL_Board board = SCL_BOARD_START_STATE;
    SCL_recordApply(r,board,100);

    uint32_t hash = SCL_boardHash32(board);

    SCL_recordInit(r);
    SCL_recordFromPGN(r,str);
    SCL_boardInit(board);
    SCL_recordApply(r,board,100);

    assert("from PGN",hash == SCL_boardHash32(board));
  }
 
  {
    puts("testing positions");

    SCL_Board board = SCL_BOARD_START_STATE;
    assert("position = normal",SCL_boardGetPosition(board) == SCL_POSITION_NORMAL);
    assert("value = 0",SCL_boardEvaluateStatic(board) == 0);

    SCL_boardSetPosition(board,
      "rnbq.rk."
      "ppppp..p"
      ".......b"
      "........"
      "..B....."
      ".N......"
      "PPPP..PP"
      "R..QKBNR",255,0,3);
    assert("position = check",SCL_boardGetPosition(board) == SCL_POSITION_CHECK);

    SCL_boardSetPosition(board,
      "rnb....q"
      "pppppQkp"
      ".......b"
      "........"
      "..B....."
      ".N......"
      "PPPP..PP"
      "R...KBNR",255,0,5);
    assert("position = mate",SCL_boardGetPosition(board) == SCL_POSITION_MATE);

    SCL_boardSetPosition(board,
      "k......."
      ".......R"
      "........"
      "........"
      "........"
      "........"
      "........"
      ".R...K..",255,0,1);
    assert("position = stalemate",SCL_boardGetPosition(board) == SCL_POSITION_STALEMATE);
    assert("value == 0",SCL_boardEvaluateStatic(board) == 0);

    SCL_boardSetPosition(board,
      "...B...."
      "......k."
      "........"
      "........"
      "K......."
      "........"
      ".....b.."
      "........",255,0,0);
    assert("position = dead",SCL_boardGetPosition(board) == SCL_POSITION_DEAD);
    assert("value = 0",SCL_boardEvaluateStatic(board) == 0);

    SCL_boardSetPosition(board,
      "....k..."
      "........"
      "........"
      "........"
      "........"
      "........"
      "..R...R."
      "....K...",255,0,0);
    assert("position = normal",SCL_boardGetPosition(board) == SCL_POSITION_NORMAL);
    assert("value != 0",SCL_boardEvaluateStatic(board) != 0);

    SCL_boardSetPosition(board,
      "rn.qkbnr"
      "pp..pppp"
      "..p....."
      "...p...."
      "........"
      "...P...."
      "PPP.PP.P"
      "R.BQKB.R",255,0,0);
    assert("position = normal",SCL_boardGetPosition(board) == SCL_POSITION_NORMAL);

    SCL_boardSetPosition(board,
      "........"
      "PPP..PPP"
      "k......."
      "...PP..."
      "........"
      "........"
      "........"
      "RNBQKBNR",255,0,5);

    assert("position = check",SCL_boardGetPosition(board) == SCL_POSITION_CHECK);
  }

  {
    puts("testing undos ");

    SCL_MoveUndo undos[1024];
    uint8_t undoCount = 0;

    SCL_Board board = SCL_BOARD_START_STATE;

    #define m(f,t)\
      undos[undoCount] = SCL_boardMakeMove(board,f,t,'q');\
      undoCount++;

    m(SCL_SQUARE('e',2),SCL_SQUARE('e',4))
    m(SCL_SQUARE('d',7),SCL_SQUARE('d',5))
    m(SCL_SQUARE('e',4),SCL_SQUARE('d',5))
    m(SCL_SQUARE('b',8),SCL_SQUARE('c',6))
    m(SCL_SQUARE('d',1),SCL_SQUARE('f',3))
    m(SCL_SQUARE('c',8),SCL_SQUARE('g',4))
    m(SCL_SQUARE('f',1),SCL_SQUARE('d',3))
    m(SCL_SQUARE('e',7),SCL_SQUARE('e',5))
    m(SCL_SQUARE('d',5),SCL_SQUARE('e',6))
    m(SCL_SQUARE('d',8),SCL_SQUARE('d',3))
    m(SCL_SQUARE('g',1),SCL_SQUARE('e',2))
    m(SCL_SQUARE('e',8),SCL_SQUARE('c',8))
    m(SCL_SQUARE('e',1),SCL_SQUARE('g',1))
    m(SCL_SQUARE('b',7),SCL_SQUARE('b',5))
    m(SCL_SQUARE('f',3),SCL_SQUARE('f',7))
    m(SCL_SQUARE('b',5),SCL_SQUARE('b',4))
    m(SCL_SQUARE('e',6),SCL_SQUARE('e',7))
    m(SCL_SQUARE('b',4),SCL_SQUARE('b',3))
    m(SCL_SQUARE('e',7),SCL_SQUARE('f',8))
    m(SCL_SQUARE('b',3),SCL_SQUARE('c',2))
    m(SCL_SQUARE('g',1),SCL_SQUARE('h',1))

    undos[undoCount] = SCL_boardMakeMove(board,
      SCL_SQUARE('c',2),
      SCL_SQUARE('b',1),'n');
    undoCount++;

    m(SCL_SQUARE('f',8),SCL_SQUARE('d',8))
    m(SCL_SQUARE('c',8),SCL_SQUARE('b',7))
    m(SCL_SQUARE('f',7),SCL_SQUARE('g',8))
    m(SCL_SQUARE('h',7),SCL_SQUARE('h',5))
    m(SCL_SQUARE('b',2),SCL_SQUARE('b',3))
    m(SCL_SQUARE('g',4),SCL_SQUARE('f',5))
    m(SCL_SQUARE('d',8),SCL_SQUARE('d',3))
    m(SCL_SQUARE('h',5),SCL_SQUARE('h',4))
    m(SCL_SQUARE('g',2),SCL_SQUARE('g',4))
    m(SCL_SQUARE('h',4),SCL_SQUARE('g',3))
    m(SCL_SQUARE('a',1),SCL_SQUARE('b',1))
    m(SCL_SQUARE('g',3),SCL_SQUARE('f',2))
    m(SCL_SQUARE('f',1),SCL_SQUARE('d',1))

    undos[undoCount] = SCL_boardMakeMove(board,
      SCL_SQUARE('f',2),
      SCL_SQUARE('f',1),'r');
    undoCount++;

    #undef m

    while (undoCount > 0)
    {
      undoCount--;
      SCL_boardUndoMove(board,undos[undoCount]);
    }

    SCL_Board board2 = SCL_BOARD_START_STATE;

    for (int i = 0; i < SCL_BOARD_STATE_SIZE; ++i)
      if (board[i] != board2[i])
      {
        puts("ERROR"); 
        return 0;
      }
   
    puts("OK"); 
  }

  {
    puts("testing AI");
      
    uint8_t s0, s1;
    char p;

    SCL_Board board = SCL_BOARD_START_STATE;
    SCL_SquareSet s = SCL_SQUARE_SET_EMPTY;

    SCL_randomSimpleSeed(40);

    printf("does AI make legal moves? ");

    for (uint16_t i = 0; i < 500; ++i)
    {
      if (SCL_boardGameOver(board))
        break;

      uint8_t depth = SCL_boardWhitesTurn(board) ? 2 : 0;
      SCL_getAIMove(board,depth,1,1,SCL_boardEvaluateStatic,SCL_randomSimple,2,
        0,0,&s0,&s1,&p);
        
      if (board[s0] == '.' || s0 == s1)
      {
        printf("ERROR: AI made an illegal move\n");

        SCL_printBoard(board,putCharacter,s,s0,SCL_PRINT_FORMAT_UTF8,4,1,0);
        return 1;
      }

      SCL_boardMakeMove(board,s0,s1,p);
    }

    puts("OK");

    printf("Does AI win against random moves? ");

    if (!(SCL_boardMate(board) && !SCL_boardWhitesTurn(board)))
    {
      printf("ERROR: AI didn't win against random moves\n");
      return 1;
    }

    puts("OK");

    /* this checks if the AI behavior changed against previous version, change
       the hash if the change was intentional */
    assert("AI behavior didn't change?",SCL_boardHash32(board) == 1629239492);

    SCL_boardFromFEN(board,"7R/8/8/1Q6/8/8/k7/3K4 w - - 0 1");

    printf("mate in 1 (white): ");

    SCL_getAIMove(board,2,3,1,SCL_boardEvaluateStatic,0,0,0,0,
      &s0,&s1,&p);
   
    if ((s0 != 63) || (s1 != 56))
    {
      puts("ERROR");
      return 1;
    }

    puts("OK");

    SCL_boardFromFEN(board,"8/5N2/8/1r5k/5q2/8/8/K7 b - - 0 1");

    printf("mate in 1 (black): ");

    SCL_getAIMove(board,2,3,1,SCL_boardEvaluateStatic,0,0,0,0,
      &s0,&s1,&p);

    if ((s0 != 29) || (s1 != 24))
    {
      puts("ERROR");
      return 1;
    }

    puts("OK");

    SCL_boardFromFEN(board,"1k2r3/8/8/7B/8/8/2N1N1N1/1K6 w - - 0 1");
    SCL_getAIMove(board,2,3,1,SCL_boardEvaluateStatic,0,0,0,0,
      &s0,&s1,&p);

    assert("makes a good move?",s0 == 39 && s1 == 60);

    SCL_getAIMove(board,2,3,1,SCL_boardEvaluateStatic,0,0,39,60,
      &s0,&s1,&p);

    assert("avoids draw?",s0 != 39 || s1 != 60);
  }

  {
    puts("testing game struct"); 

    SCL_Game game;

    SCL_gameInit(&game,0);

    SCL_gameMakeMove(&game,1,18,'q');
    SCL_gameMakeMove(&game,57,40,'q');
    SCL_gameMakeMove(&game,18,1,'q');
    SCL_gameMakeMove(&game,40,57,'q');

    SCL_gameMakeMove(&game,1,18,'q');
    SCL_gameMakeMove(&game,57,40,'q');
    SCL_gameMakeMove(&game,18,1,'q');

    assert("game state == playing?",game.state == SCL_GAME_STATE_PLAYING);

    SCL_gameUndoMove(&game);
    SCL_gameMakeMove(&game,18,1,'q');
    SCL_gameMakeMove(&game,40,57,'q');

    assert("repetition draw?",game.state == SCL_GAME_STATE_DRAW_REPETITION);
  }

  return 0;

  #undef assert
}
