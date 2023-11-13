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

#include "complex.h"

#define Mag(c)  ((c)->a*(c)->a + (c)->b*(c)->b)

/*****************************************************************************/

static int
complex_abs_cmp_internal(Complex *a, Complex *b)
{
    double      amag = Mag(a),
                bmag = Mag(b);

    if (amag < bmag)
        return -1;
    if (amag > bmag)
        return 1;
    return 0;
}

PG_FUNCTION_INFO_V1(complex_abs_eq);
Datum
complex_abs_eq(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) == 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_ne);
Datum
complex_abs_ne(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) != 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_lt);
Datum
complex_abs_lt(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) < 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_le);
Datum
complex_abs_le(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) <= 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_gt);
Datum
complex_abs_gt(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) > 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_ge);
Datum
complex_abs_ge(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_abs_cmp_internal(c, d) >= 0;
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_abs_cmp);
Datum
complex_abs_cmp(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  int result = complex_abs_cmp_internal(c, d);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_INT32(result);
}

/*****************************************************************************/
