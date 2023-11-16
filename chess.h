/*
 * complex.h
 */

/* Structure to represent complex numbers */
typedef struct
{
  double    a,
            b;
} Complex;

/* deine one move in SAN */
typedef struct
{
  int       piece;
  char      file;
  int       rank;
  char      from_file;
  int       from_rank;
  bool      capture;
  bool      promotion;
  bool      check;
  bool      checkmate;
  bool      drawoffer;
  bool      castling;
} SANMove;

/* define SAN */
typedef struct
{
  // dynamic list of SANmoves every row contains 2 moves for white and black
  SANMove  **moves;
  int       size;
} Chessgame;


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
