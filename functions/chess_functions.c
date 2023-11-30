#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "smallchesslib.h"
#include "chess_functions.h"

PG_FUNCTION_INFO_V1(getBoard);

// **********************************
//getBoard 
Datum getBoard(PG_FUNCTION_ARGS) {
    // Check and extract function arguments
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    // Get the chess game and half-move count parameters
    text *chessgame_text = PG_GETARG_TEXT_P(0);
    int32 half_moves = PG_GETARG_INT32(1);

    // Convert text input to C string
    char *chessgame_str = text_to_cstring(chessgame_text);

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


// ********************************
// getFirstMoves

PG_FUNCTION_INFO_V1(getFirstMoves);

Datum getFirstMoves(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    text *chessgame_text = PG_GETARG_TEXT_P(0);
    int32 moves_count = PG_GETARG_INT32(1);

    if (moves_count < 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid move count")));
    }

    char *chessgame_str = text_to_cstring(chessgame_text);

    // Truncate the chess game to the specified number of half-moves using smallchesslib
    char truncated_chessgame[MAX_LENGTH]; // Buffer to store the truncated chess game
    int result = smallchesslib_truncate_game(chessgame_str, moves_count, truncated_chessgame);

    if (result != 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Error truncating chess game")));
    }

    text *truncated_chessgame_text = cstring_to_text(truncated_chessgame);

    PG_RETURN_TEXT_P(truncated_chessgame_text);
}

// **************************************************
// hasOpening 

PG_FUNCTION_INFO_V1(hasOpening);

Datum hasOpening(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    text *chessgame1_text = PG_GETARG_TEXT_P(0);
    text *chessgame2_text = PG_GETARG_TEXT_P(1);

    char *chessgame1_str = text_to_cstring(chessgame1_text);
    char *chessgame2_str = text_to_cstring(chessgame2_text);

    // Check if the first chess game starts with the exact moves of the second chess game
    bool has_same_opening = smallchesslib_compare_opening(chessgame1_str, chessgame2_str);

    PG_RETURN_BOOL(has_same_opening);
}


// **************************************************
// hasBoard 
PG_FUNCTION_INFO_V1(hasBoard);

Datum hasBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    text *chessgame_text = PG_GETARG_TEXT_P(0);
    text *chessboard_text = PG_GETARG_TEXT_P(1);
    int32 half_moves = PG_GETARG_INT32(2);

    if (half_moves < 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    }

    char *chessgame_str = text_to_cstring(chessgame_text);
    char *chessboard_str = text_to_cstring(chessboard_text);

    // Check if the chess game contains the given board state in its initial N half-moves
    bool contains_board_state = smallchesslib_contains_board_state(chessgame_str, chessboard_str, half_moves);

    PG_RETURN_BOOL(contains_board_state);
}