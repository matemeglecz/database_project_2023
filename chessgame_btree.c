/*
 * complex_abs.C 
 *
 * Btree functions for the Complex type
 *
 */

#include <postgres.h>
#include <float.h>
#include <math.h>

#include "fmgr.h"
#include "utils/builtins.h"
#include "libpq/pqformat.h"

#include "chess.h"


/*****************************************************************************/

static int
chessgame_cmp_internal(Chessgame *c1, Chessgame *c2)
{
    bool c1_in_c2 = hasOpeningInternal(c2->game, c1->game);
    bool c2_in_c1 = hasOpeningInternal(c1->game, c2->game);
    if (c1_in_c2 && c2_in_c1) {
        return 0;
    } else if (c1_in_c2) {
        return -1;
    } else if (c2_in_c1) {
        return 1;
    } else {
        return 0;
    }
}

PG_FUNCTION_INFO_V1(chessgame_eq);
Datum
chessgame_eq(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c1, c2) == 0;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_ne);
Datum
chessgame_ne(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c1, c2) != 0;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_lt);
Datum
chessgame_lt(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c2, c1) == -1;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_le);
Datum
chessgame_le(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c2, c1) <= 0;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_gt);
Datum
chessgame_gt(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c1, c2) == 1;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_ge);
Datum
chessgame_ge(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    bool result = chessgame_cmp_internal(c1, c2) >= 0;
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(chessgame_cmp);
Datum
chessgame_cmp(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    int result = chessgame_cmp_internal(c1, c2);
    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_INT32(result);
}



/*****************************************************************************/
