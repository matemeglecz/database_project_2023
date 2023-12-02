
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




