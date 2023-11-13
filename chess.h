/*
 * complex.h
 */

/* Structure to represent complex numbers */
typedef struct
{
  double    a,
            b;
} Complex;

/* fmgr macros complex type */

#define DatumGetComplexP(X)  ((Complex *) DatumGetPointer(X))
#define ComplexPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_COMPLEX_P(n) DatumGetComplexP(PG_GETARG_DATUM(n))
#define PG_RETURN_COMPLEX_P(x) return ComplexPGetDatum(x)

/*****************************************************************************/

#define EPSILON         1.0E-06

#define FPzero(A)       (fabs(A) <= EPSILON)
#define FPeq(A,B)       (fabs((A) - (B)) <= EPSILON)
#define FPne(A,B)       (fabs((A) - (B)) > EPSILON)
#define FPlt(A,B)       ((B) - (A) > EPSILON)
#define FPle(A,B)       ((A) - (B) <= EPSILON)
#define FPgt(A,B)       ((A) - (B) > EPSILON)
#define FPge(A,B)       ((B) - (A) <= EPSILON)

/*****************************************************************************/
