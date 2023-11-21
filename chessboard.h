#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <stdbool.h>

/* Structure to represent a chess board in FEN notation */
typedef struct
{
    char data[64];  // 8x8 chess board represented as a linear array
} ChessBoard;

/* fmgr macros for chessboard type */
#define DatumGetChessBoardP(X)  ((ChessBoard *) DatumGetPointer(X))
#define ChessBoardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetChessBoardP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSBOARD_P(x) return ChessBoardPGetDatum(x)

/*****************************************************************************/
/* Function declarations for input/output and conversion */

/* Input function */
extern Datum chessboard_in(PG_FUNCTION_ARGS);

/* Output function */
extern Datum chessboard_out(PG_FUNCTION_ARGS);

/* Receive function */
extern Datum chessboard_recv(PG_FUNCTION_ARGS);

/* Send function */
extern Datum chessboard_send(PG_FUNCTION_ARGS);

/* Constructor function */
extern Datum chessboard_constructor(PG_FUNCTION_ARGS);

/* Accessor functions */
extern Datum chessboard_get_piece(PG_FUNCTION_ARGS);
extern Datum chessboard_set_piece(PG_FUNCTION_ARGS);
extern Datum chessboard_clear_board(PG_FUNCTION_ARGS);

/* Utility functions */
extern char* get_fen_notation(ChessBoard* board);
extern ChessBoard* set_board_from_fen(const char* fen);

#endif /* CHESSBOARD_H */
