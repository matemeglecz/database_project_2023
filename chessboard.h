#ifndef CHESSBOARD_H
#define CHESSBOARD_H
#include "string.h"

/* Structure to represent a chess board */
typedef struct
{
    string data[64];  // 8x8 chess board represented as a linear array
    
} ChessBoard;

/* Function manager macros for chessboard type */

#define DatumGetChessBoardP(X)  ((ChessBoard *) DatumGetPointer(X))
#define ChessBoardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetChessBoardP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSBOARD_P(x) return ChessBoardPGetDatum(x)

#endif /* CHESSBOARD_H */
