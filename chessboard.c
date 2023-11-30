#include "chessboard.h"
#include "utils/builtins.h"
#include "utils/fmgrprotos.h"
#include "libpq/pqformat.h"
#include <string.h>

PG_MODULE_MAGIC;

/*****************************************************************************/

static Chessboard *
chessboard_make(char *fen) {
    // Vous devez implémenter la logique pour créer un plateau d'échecs à partir de la notation FEN
    // Utilisez la structure Chessboard et remplissez-la en fonction de la notation FEN.
    // ...
    
    // Exemple de création d'un plateau avec une pièce à la position (0, 0)
    Chessboard *result = (Chessboard *)palloc0(sizeof(Chessboard));
    int rank = 7;
    int file = 0;

    // position of pieces
    while (*fen && rank >= 0) {
        if (*fen == ' ') {
            // Ignore spaces
            fen++;
        } else if (*fen == '/') {
            // Move to the next rank
            rank--;
            file = 0;
            fen++;
        } else if (*fen >= '1' && *fen <= '8') {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; i++) {
                result->board[rank * 8 + file] = '0';
                file++;
            }
            fen++;
        } else {
            // Piece
            result->board[rank * 8 + file] = *fen;
            file++;
            fen++;
        }
    }

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        // FEN is incomplete or malformed
        fprintf(stderr, "Invalid FEN string\n");
        exit(EXIT_FAILURE);
    }

    // Current color
    fen++;
    result->currentColor = *fen;

    // Move to the next part of FEN
    fen = strchr(fen, ' ');

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        // FEN is incomplete or malformed
        fprintf(stderr, "Invalid FEN string\n");
        exit(EXIT_FAILURE);
    }

    // Castling availability
    fen++;
    strncpy(result->castling, fen, sizeof(result->castling));

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
        // FEN is incomplete or malformed
        fprintf(stderr, "Invalid FEN string\n");
        exit(EXIT_FAILURE);
    }

    // Half-move clock
    fen++;
    result->halfMoveClock = atoi(fen);

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        // FEN is incomplete or malformed
        fprintf(stderr, "Invalid FEN string\n");
        exit(EXIT_FAILURE);
    }

    // Full-move number
    fen++;
    result->fullMoveNumber = atoi(fen);

    return result;
    
}

/*****************************************************************************/

static Chessboard * chessboard_parse(char* fen){
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
    Chessboard *chessboard = (Chessboard *)PG_GETARG_CHESSBOARD_P(0);
    char *result = complex_to_str(chessboard);
    PG_FREE_IF_COPY(chessboard, 0);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS) {
    StringInfoData buf;
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(0);

    pq_begintypsend(&buf);

    pq_sendbytes(&buf, chessboard->board, sizeof(chessboard->board));

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

    // Ajoutez ici le code pour recevoir chaque membre de la structure Chessboard
    // Utilisez les fonctions pq_recv* pour chaque membre

    pq_copymsgbytes(buf, result->board, sizeof(result->board));

    result->currentColor = pq_getmsgbyte(buf);

    pq_copymsgstring(buf, result->castling, sizeof(result->castling));

    pq_copymsgstring(buf, result->enPassant, sizeof(result->enPassant));

    result->halfMoveClock = pq_getmsgint(buf, sizeof(result->halfMoveClock));
    result->fullMoveNumber = pq_getmsgint(buf, sizeof(result->fullMoveNumber));

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
    Chessboard *result = chessboard_make(fen);
    PG_RETURN_CHESSBOARD_P(result);
}
/*****************************************************************************/
