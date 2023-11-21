#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include "chessboard.h"

PG_MODULE_MAGIC;

/*****************************************************************************/
/* Function declarations for input/output and conversion */

PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_constructor);
Datum chessboard_constructor(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_get_piece);
Datum chessboard_get_piece(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_set_piece);
Datum chessboard_set_piece(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(chessboard_clear_board);
Datum chessboard_clear_board(PG_FUNCTION_ARGS);

/*****************************************************************************/
/* Function definitions */


static ChessBoard *
chessboard_make(char* fen)
{
    ChessBoard *cb = palloc(sizeof(ChessBoard));

    // copy the FEN string to the board
    strncpy(cb->data, fen, sizeof(cb->data) - 1);
    cb->data[sizeof(cb->data) - 1] = '\0';

    // Additional validation logic can be added here if needed

    return cb;
}

/* Input function */
Datum
chessboard_in(PG_FUNCTION_ARGS)
{
    char* str = PG_GETARG_CSTRING(0);
    ChessBoard* board = set_board_from_fen(str);
    PG_RETURN_CHESSBOARD_P(board);
}

/* Output function */
Datum
chessboard_out(PG_FUNCTION_ARGS)
{
    ChessBoard* board = PG_GETARG_CHESSBOARD_P(0);
    char* result = get_fen_notation(board);
    PG_RETURN_CSTRING(result);
}

/* Receive function */
Datum
chessboard_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
    ChessBoard* board = (ChessBoard *) palloc(sizeof(ChessBoard));
    memcpy(board, pq_getmsgbytes(buf, sizeof(ChessBoard)), sizeof(ChessBoard));
    PG_RETURN_CHESSBOARD_P(board);
}

/* Send function */
Datum
chessboard_send(PG_FUNCTION_ARGS)
{
    ChessBoard* board = PG_GETARG_CHESSBOARD_P(0);
    StringInfoData buf;
    pq_begintypsend(&buf);
    pq_sendbytes(&buf, (char *)board, sizeof(ChessBoard));
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Constructor function */
Datum
chessboard_constructor(PG_FUNCTION_ARGS)
{
    char* fen = PG_GETARG_CSTRING(0);
    ChessBoard* board = set_board_from_fen(fen);
    PG_RETURN_CHESSBOARD_P(board);
}

/* Accessor function */
Datum
chessboard_get_piece(PG_FUNCTION_ARGS)
{
    ChessBoard* board = PG_GETARG_CHESSBOARD_P(0);
    int rank = PG_GETARG_INT32(1);
    int file = PG_GETARG_INT32(2);

    // Perform bounds checking
    if (rank < 1 || rank > 8 || file < 1 || file > 8)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("Invalid board coordinates")));

    char piece = board->data[(rank - 1) * 8 + (file - 1)];
    PG_RETURN_CHAR(piece);
}

/* Modifier function */
Datum
chessboard_set_piece(PG_FUNCTION_ARGS)
{
    ChessBoard* board = PG_GETARG_CHESSBOARD_P(0);
    int rank = PG_GETARG_INT32(1);
    int file = PG_GETARG_INT32(2);
    char piece = PG_GETARG_CHAR(3);

    // Perform bounds checking
    if (rank < 1 || rank > 8 || file < 1 || file > 8)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("Invalid board coordinates")));

    // Validate piece
    if (!(piece == '0' || (piece >= 'a' && piece <= 'z') || (piece >= 'A' && piece <= 'Z')))
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                        errmsg("Invalid chess piece")));

    // Set the piece on the board
    board->data[(rank - 1) * 8 + (file - 1)] = piece;

    PG_RETURN_CHESSBOARD_P(board);
}

/* Utility function to clear the board */
Datum
chessboard_clear_board(PG_FUNCTION_ARGS)
{
    ChessBoard* board = PG_GETARG_CHESSBOARD_P(0);

    // Clear the board
    for (int i = 0; i < 64; ++i)
        board->data[i] = '0';

    PG_RETURN_CHESSBOARD_P(board);
}

/* Utility function to get FEN notation from ChessBoard */
char*
get_fen_notation(ChessBoard* board)
{
    char* result = palloc(sizeof(char) * 80);
    int index = 0;
    int emptyCount = 0;

    for (int rank = 8; rank >= 1; --rank)
    {
        for (int file = 1; file <= 8; ++file)
        {
            char piece = board->data[(rank - 1) * 8 + (file - 1)];

            if (piece == '0')
            {
                // Empty square
                ++emptyCount;
            }
            else
            {
                // Piece found
                if (emptyCount > 0)
                {
                    result[index++] = emptyCount + '0';
                    emptyCount = 0;
                }
                result[index++] = piece;
            }
        }

        if (emptyCount > 0)
        {
            result[index++] = emptyCount + '0';
            emptyCount = 0;
        }

        if (rank > 1)
            result[index++] = '/';
    }

    result[index] = '\0';

    return result;
}

/* Utility function to set ChessBoard from FEN notation */
ChessBoard*
set_board_from_fen(const char* fen)
{
    ChessBoard* board = palloc(sizeof(ChessBoard));
    int index = 0;
    int rank = 8;
    int file = 1;

    while (*fen != '\0')
    {
        if (*fen >= '1' && *fen <= '8')
        {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; ++i)
                board->data[index++] = '0';
            file += count;
        }
        else if (*fen == '/')
        {
            // Move to the next rank
            --rank;
            file = 1;
        }
        else
        {
            // Piece
            board->data[index++] = *fen;
            ++file;
        }

        ++fen;
    }

    return board;
}
