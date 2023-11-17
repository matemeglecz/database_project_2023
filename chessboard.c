#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

#include "chessBoard.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(chessboard_constructor);
PG_FUNCTION_INFO_V1(chessboard_get_piece);
PG_FUNCTION_INFO_V1(chessboard_get_row);
PG_FUNCTION_INFO_V1(chessboard_in);
PG_FUNCTION_INFO_V1(chessboard_out);
PG_FUNCTION_INFO_V1(chessboard_send);
PG_FUNCTION_INFO_V1(chessboard_recv);

Datum chessboard_constructor(PG_FUNCTION_ARGS) {
    text *input = PG_GETARG_TEXT_P(0);
    char *input_str = text_to_cstring(input);

    // Assurez-vous que la chaîne a la longueur attendue (64 pour un échiquier 8x8)
    if (strlen(input_str) != 64) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid ChessBoard string")));
    }

    ChessBoard *result = (ChessBoard *) palloc(sizeof(ChessBoard));

    // Remplir le tableau bidimensionnel
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result->data[i][j] = input_str[i * 8 + j];
        }
    }

    PG_RETURN_POINTER(result);
}

Datum chessboard_get_piece(PG_FUNCTION_ARGS) {
    ChessBoard *board = (ChessBoard *) PG_GETARG_POINTER(0);
    int32 row = PG_GETARG_INT32(1);
    int32 col = PG_GETARG_INT32(2);

    // Assurez-vous que les indices sont valides
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid indices for ChessBoard")));
    }

    char piece = board->data[row][col];

    PG_RETURN_CHAR(piece);
}

Datum chessboard_get_row(PG_FUNCTION_ARGS) {
    ChessBoard *board = (ChessBoard *) PG_GETARG_POINTER(0);
    int32 row = PG_GETARG_INT32(1);

    // Assurez-vous que la ligne est valide
    if (row < 0 || row >= 8) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid row for ChessBoard")));
    }

    text *result = palloc(VARHDRSZ + 9);  // Longueur maximale d'une ligne + en-tête de texte
    SET_VARSIZE(result, VARHDRSZ + 8);
    memcpy(VARDATA(result), board->data[row], 8);

    PG_RETURN_TEXT_P(result);
}

Datum chessboard_in(PG_FUNCTION_ARGS) {
    char *input_str = PG_GETARG_CSTRING(0);
    ChessBoard *result = (ChessBoard *) palloc(sizeof(ChessBoard));

    // Assurez-vous que la chaîne a la longueur attendue (64 pour un échiquier 8x8)
    if (strlen(input_str) != 64) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid ChessBoard string")));
    }

    // Remplir le tableau bidimensionnel
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result->data[i][j] = input_str[i * 8 + j];
        }
    }

    PG_RETURN_POINTER(result);
}

Datum chessboard_out(PG_FUNCTION_ARGS) {
    ChessBoard *board = (ChessBoard *) PG_GETARG_POINTER(0);
    char *output_str = palloc(65);  // 64 caractères + 1 pour le terminateur de chaîne

    // Convertir le tableau bidimensionnel en une chaîne de caractères
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            output_str[i * 8 + j] = board->data[i][j];
        }
    }

    output_str[64] = '\0';

    PG_RETURN_CSTRING(output_str);
}

Datum chessboard_send(PG_FUNCTION_ARGS) {
    ChessBoard *board = (ChessBoard *) PG_GETARG_POINTER(0);
    bytea *result;

    result = (bytea *) palloc(VARHDRSZ + sizeof(ChessBoard));
    SET_VARSIZE(result, VARHDRSZ + sizeof(ChessBoard));
    memcpy(VARDATA(result), board, sizeof(ChessBoard));

    PG_RETURN_BYTEA_P(result);
}

Datum chessboard_recv(PG_FUNCTION_ARGS) {
    bytea *input = PG_GETARG_BYTEA_P(0);
    ChessBoard *result;

    if (VARSIZE(input) != VARHDRSZ + sizeof(ChessBoard)) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid ChessBoard bytea")));
    }

    result = (ChessBoard *) palloc(sizeof(ChessBoard));
    memcpy(result, VARDATA(input), sizeof(ChessBoard));

    PG_RETURN_POINTER(result);
}