/*
 * complex.h
 */

/* Structure to represent complex numbers */
typedef struct
{
  double    a,
            b;
} Complex;

/* deine one move in SAN */
typedef struct
{
  char       piece;
  char      file;
  int       rank;
  char      from_file;
  int       from_rank;
  bool      capture;
  bool      promotion;
  bool      check;
  bool      checkmate;
  bool      drawoffer;
  bool      castle;
} SANmove;

/* define SAN */
typedef struct
{
  // dynamic list of SANmoves every row contains 2 moves for white and black
  SANmove  **moves;
  int       size;
} Chessgame_helper;

typedef struct 
{
  char game[5000];
} Chessgame;




/* fmgr macros complex type */

#define DatumGetComplexP(X)  ((Complex *) DatumGetPointer(X))
#define ComplexPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_COMPLEX_P(n) DatumGetComplexP(PG_GETARG_DATUM(n))
#define PG_RETURN_COMPLEX_P(x) return ComplexPGetDatum(x)

#define DatumGetChessgameP(X)  ((Chessgame *) DatumGetPointer(X))
#define ChessgamePGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSGAME_P(n) DatumGetChessgameP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSGAME_P(x) return ChessgamePGetDatum(x)

/*****************************************************************************/


typedef struct
{
    char board[64];        // 8x8 chess board represented as a linear array
    char currentColor;     // Current turn: 'w' for WHITE, 'b' for BLACK
    char castling[5];      // Castling availability, e.g., "KQkq"
    char enPassant[2];      // Square of the possible en passant target (file, rank), e.g., {2, 5}
    int halfMoveClock;     // Half-move clock for the 50-move rule
    int fullMoveNumber;    // Full-move number
} Chessboard;

#define DatumGetChessboardP(X)  ((Chessboard *) DatumGetPointer(X))
#define ChessboardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetChessboardP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSBOARD_P(x) return ChessboardPGetDatum(x)




