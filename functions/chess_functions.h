#ifndef CHESS_FUNCTIONS_H
#define CHESS_FUNCTIONS_H

#include "postgres.h"
#include "fmgr.h"

Datum getBoard(PG_FUNCTION_ARGS);
Datum getFirstMoves(PG_FUNCTION_ARGS);
Datum hasOpening(PG_FUNCTION_ARGS);
Datum hasBoard(PG_FUNCTION_ARGS);

#endif  /* CHESS_FUNCTIONS_H */


