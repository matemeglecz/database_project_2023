#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/fmgrprotos.h"
#include "libpq/pqformat.h"
#include <string.h>
#include "chess.h"
#include "smallchesslib/smallchesslib.h"


//PG_MODULE_MAGIC;


/*****************************************************************************/

static Chessboard *
chessboard_make(char *fen) {

    Chessboard *result = palloc(sizeof(Chessboard));
    // int rank = 7;
    // int file = 0;
    int index = 0;
    if(result == NULL)
    {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("1 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
      // Parse the piece positions
    while (*fen && *fen != ' ') {
        if (*fen == '/') {
            fen++;
        } else if (*fen >= '1' && *fen <= '8') {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; i++) {
                result->board[index] = '0';
                index++;
            }
            fen++;
        } else {
            // Piece
            result->board[index] = *fen;
            index++;
            fen++;
        }
    }

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        // FEN is incomplete or malformed
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("2 invalid input syntax for type %s:  \"%s\"", "chessboard", fen)));
    }

    // Current color
    fen++;
    result->currentColor = *fen;

    // Move to the next part of FEN
    fen = strchr(fen, ' ');

    if (!fen) {
       ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("3 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Castling availability

    fen++;
    size_t castlingLength = strcspn(fen, " ");
    if (castlingLength > sizeof(result->castling) ) {
        castlingLength = sizeof(result->castling);
    }
    strncpy(result->castling, fen, castlingLength);

    result->castling[castlingLength] = '\0';
    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("6 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
    // En passant target square
    fen++;
    if (*fen != '-') {
        result->enPassant[0] = *fen;
        fen++;
        result->enPassant[1] = *fen;
    }else{
        result->enPassant[0] = *fen;
    }

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("4 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Half-move clock
    fen++;
    
    result->halfMoveClock = atoi(fen);

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
       ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("5 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Full-move number
    fen++;
    result->fullMoveNumber = atoi(fen);

    return result;
    
}

/*****************************************************************************/

static Chessboard * chessboard_parse(char *fen){
    if(!fen){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    errmsg("parse invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
    
    return chessboard_make(fen);
}

static char* chessboard_to_str(const Chessboard* board){
    char* fen = (char*)malloc(sizeof(char) * 100);
    int index = 0;
    int emptyCount = 0;
    for(int i = 63; i >= 0; i--){
        if(board->board[i] != '0'){
            if(emptyCount > 0){
                fen[index++] = emptyCount + '0';
                emptyCount = 0;
            }
            fen[index++] = board->board[i];
            
        }else{
            emptyCount++;
        }
        //new file
        if(i % 8 == 0){
            if(emptyCount > 0)
                fen[index++] = emptyCount + '0';
            if(i > 0)
                fen[index++] = '/';
            emptyCount = 0;
        }

    }

    fen[index++] = ' ';
    fen[index++] = board->currentColor;
    fen[index++] = ' ';

    int i = 0;
    while(board->castling[i] != '\0'){
        fen[index++] = board->castling[i];
        i++;
    }

    fen[index++] = ' ';

    // En passant target square
    if (board->enPassant[0] == '-') {
        fen[index++] = '-';
    } else {
        fen[index++] = board->enPassant[0];
        fen[index++] = board->enPassant[1];
    }

    fen[index++] = ' ';
    sprintf(&fen[index], "%d %d", board->halfMoveClock, board->fullMoveNumber);

    return fen;
}


/*****************************************************************************/


PG_FUNCTION_INFO_V1(chessboard_in);
Datum chessboard_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    
    Chessboard *result = chessboard_make(str);
    PG_RETURN_CHESSBOARD_P(result);
}

PG_FUNCTION_INFO_V1(chessboard_out);
Datum chessboard_out(PG_FUNCTION_ARGS) {
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(0);
    char *result = palloc(sizeof(char) * strlen(chessboard_to_str(chessboard)));
    strcpy(result, chessboard_to_str(chessboard));
    PG_FREE_IF_COPY(chessboard, 0);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS) {
    StringInfoData buf;
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(0);

    pq_begintypsend(&buf);

    pq_sendstring(&buf, chessboard->board);

    pq_sendbyte(&buf, chessboard->currentColor);

    pq_sendstring(&buf, chessboard->castling);

    pq_sendstring(&buf, chessboard->enPassant);

    pq_sendint(&buf, chessboard->halfMoveClock, sizeof(chessboard->halfMoveClock));
    pq_sendint(&buf, chessboard->fullMoveNumber, sizeof(chessboard->fullMoveNumber));
    PG_FREE_IF_COPY(chessboard, 0);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessboard_recv);
Datum chessboard_recv(PG_FUNCTION_ARGS) {
    StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
    Chessboard *result;

    result = (Chessboard *)palloc(sizeof(Chessboard));
    result = chessboard_parse(pq_getmsgstring(buf));
    // memcpy(result->board, pq_getmsgstring(buf, sizeof(result->board)), sizeof(result->board));

    // result->currentColor = pq_getmsgbyte(buf);

    // strncpy(result->castling, pq_getmsgstring(buf), sizeof(result->castling));
    // strncpy(result->enPassant, pq_getmsgstring(buf), sizeof(result->enPassant));

    // result->halfMoveClock = pq_getmsgint(buf, sizeof(result->halfMoveClock));
    // result->fullMoveNumber = pq_getmsgint(buf, sizeof(result->fullMoveNumber));

    PG_RETURN_CHESSBOARD_P(result);
}

PG_FUNCTION_INFO_V1(chessboard_cast_from_text);
Datum
chessboard_cast_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = DatumGetCString(DirectFunctionCall1(textout,
               PointerGetDatum(txt)));
  PG_RETURN_CHESSBOARD_P(chessboard_make(str));
}

PG_FUNCTION_INFO_V1(chessboard_cast_to_text);
Datum
chessboard_cast_to_text(PG_FUNCTION_ARGS)
{
  Chessboard *c  = PG_GETARG_CHESSBOARD_P(0);
  text *out = (text *)DirectFunctionCall1(textin,
            PointerGetDatum(chessboard_to_str(c)));
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_TEXT_P(out);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(chessboard_constructor);
Datum chessboard_constructor(PG_FUNCTION_ARGS) {
    char* fen = PG_GETARG_CSTRING(0);
    if(!fen){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    errmsg("constructor invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
    
    Chessboard *result = chessboard_parse(fen);
    PG_RETURN_CHESSBOARD_P(result);
}
/*****************************************************************************/

PG_FUNCTION_INFO_V1(getBoard);
Datum getBoard(PG_FUNCTION_ARGS) {
    // Check and extract function arguments
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    // Get the chess game and half-move count parameters
    Chessgame *chessgame = PG_GETARG_CHESSGAME_P(0);
    int32 half_moves = PG_GETARG_INT32(1);

    // Convert text input to C string
    char *chessgame_str = chessgame_to_str(chessgame);

    // Use smallchesslib functions to process the chess game
    // Example: fetch the board state at the specified half-move
    char fen[128]; // Buffer to store the FEN representation of the board state
    // Example usage of smallchesslib, replace with appropriate functions from the library
    int result = smallchesslib_get_board_state(chessgame_str, half_moves, fen);

    // Check for errors or invalid moves
    if (result != 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    }

    // Convert the FEN string to a PostgreSQL text type
    text *fen_text = cstring_to_text(fen);

    // Return the FEN representation of the board state
    PG_RETURN_TEXT_P(fen_text);
}

PG_FUNCTION_INFO_V1(hasBoard);
Datum hasBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    Chessgame *chessgame = PG_GETARG_CHESSGAME_P(0);
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(1);
    int32 half_moves = PG_GETARG_INT32(2);

    // Convert text input to C string
    char *chessgame_text = chessgame_to_str(chessgame);
     char *chessboard_text = chessboard_to_str(chessboard);

    if (half_moves < 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    }

    char *chessgame_str = text_to_cstring(chessgame_text);
    char *chessboard_str = text_to_cstring(chessboard_text);

    // Check if the chess game contains the given board state in its initial N half-moves
    bool contains_board_state = smallchesslib_contains_board_state(chessgame_str, chessboard_str, half_moves);

    PG_RETURN_BOOL(contains_board_state);
}
