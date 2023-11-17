/*
 * complex.C 
 *
 * PostgreSQL Complex Number Type:
 *
 * complex '(a,b)'
 *
 * Author: Maxime Schoemans <maxime.schoemans@ulb.be>
 */

#include <stdio.h>
#include <postgres.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "utils/builtins.h"
#include "libpq/pqformat.h"

#include "chess.h"

PG_MODULE_MAGIC;

char *strndup(char *str, int chars)
{
    char *buffer;
    int n;

    buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

/*****************************************************************************/

static Complex *
complex_make(double a, double b)
{
  Complex *c = palloc(sizeof(Complex));
  c->a = a;
  c->b = b;
  /* this is not superflous code, we need
  to account for negative zeroes here */
  if (c->a == 0)
    c->a = 0;
  if (c->b == 0)
    c->b = 0;
  return c;
}

static Chessgame *
chessgame_make(SANmove **moves, int size)
{
  Chessgame *c = malloc(sizeof(Chessgame));
  c->moves = moves;
  c->size = size;
  return c;
}

/*****************************************************************************/

static SANmove
sanmove_parse(char *in)
{
  SANmove c;
  // initialize all the booleans to false
    c.capture = false;
    c.promotion = false;
    c.check = false;
    c.checkmate = false;
    c.drawoffer = false;
    c.castle = false;
    // initialize all the chars to 0
    c.piece = '0';
    c.file = '0';
    c.rank = 0;
    c.from_file = '0';
    c.from_rank = 0;
  //copy the input string to a new string
    char *str = malloc(sizeof(in));
    strcpy(str, in);
  //split the input into substrings devided by spaces

    //if the substring is a number, it is the move number
    if (atoi(in) != 0)
    {
      return c;
    }
    //if the first chararacter is K or Q or R or B or N, store the the piece else store p in piece we will call pawns p
    if (in[0] == 'K' || in[0] == 'Q' || in[0] == 'R' || in[0] == 'B' || in[0] == 'N')
    {
      c.piece = in[0];
    }
    else
    {
      c.piece = 'p';
    }
    // if the substring contains an x, it is a capture
    if (strchr(in, 'x') != NULL)
    {
      c.capture = true;
    }
    //if the substring contains a +, it is a check
    if (strchr(in, '+') != NULL)
    {
      c.check = true;
    }
    //if the substring contains a #, it is a mate
    if (strchr(in, '#') != NULL)
    {
      c.checkmate = true;
    }
    //if the substring contains a = or a '(' and ')' or a '/', it is a promotion
    if (strchr(in, '=') != NULL || (strchr(in, '(') != NULL && strchr(in, ')') != NULL) || strchr(in, '/') != NULL)
    {
      c.promotion = true;
    }
    //if the substring contains a 0-0 or 0-0-0 or O-O or O-O-O, it is a castle
    if (strstr(in, "0-0") != NULL || strstr(in, "O-O") != NULL)
    {
      c.castle = true;
    }
    //if the substring contains a =, it is a drawoffer
    if (strchr(in, '=') != NULL)
    {
      c.drawoffer = true;
    }

    //iterate through the characters of the substring backwards
    bool file_found = false;
    bool rank_found = false;
    for (int i = strlen(in) - 1; i >= 0; i--)
    {
      //if the character is a number, it is the rank
      if (atoi(&in[i]) >= 1 && atoi(&in[i]) <= 8)
      {
        if (rank_found==false){
          c.rank = atoi(&in[i]);
          rank_found = true;
        } else
        {
          c.from_rank = atoi(&in[i]);
        }
      }
      //if the character is a letter, it is the file
      if (in[i] >= 'a' && in[i] <= 'h')
      {
        if(file_found==false){
          c.file = in[i];
          file_found = true;
        } else
        {
          c.from_file = in[i];
        }
      }
    }


  return c;
}

static Chessgame *
chessgame_parse(char in[])
{
  //remove line breaks
  char* str = strtok(in, "\n");
  // steps in game
  int step = 0;
  //create a 2 dim dynamic array for storing all the move pairs 2 by 100
  SANmove **allmoves = (SANmove **)malloc(sizeof(SANmove*) * 100);
  for (int i = 0; i < 100; ++i)
  {
      allmoves[i] = (SANmove *)malloc(2 * sizeof(SANmove));

  }

  //iterate the str until a '.' is found, and process the string between the two dots, the this for the whole string
  while (str != NULL)
  {
    char *dot = strchr(str, '.');
    if (dot != NULL)
    {
      char *dot2 = strchr(dot + 1, '.');
        bool end_reached = false;
      while (dot != NULL && !end_reached)
      {
        // create an array to store 2 sanmoves
        SANmove sanmoves[2];
        // if dot2 and dot is the same than dot2 is the end of the string
        if (dot2 == NULL)
        {
          dot2 = strchr(dot + 1, '\0');
            end_reached = true;
        }
        char san[dot2 - dot];
        strcpy(san, strndup(dot + 1, dot2 - dot - 1));
          san[dot2 - dot - 1] = '\0';
        // split by spaces and process san with sanmove_parse each substring
        char delim[] = " ";
        char *token = strtok(san, delim);
        int moves = 0;
        while (token != NULL && moves < 2)
        {
          sanmoves[moves] = sanmove_parse(token);
          token = strtok(NULL, delim);
          moves++;
        }
        // store the sanmove in allmoves
      allmoves[step][0] = sanmoves[0];
      allmoves[step][1] = sanmoves[1];
        dot = dot2 + 1;
        dot2 = strchr(dot, '.');
      step++;
      }
    }
    str = strtok(NULL, "\n");

  }

/*
  if (sscanf(str, " ( %lf , %lf )", &a, &b) != 2)
    ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("invalid input syntax for type %s: \"%s\"", "complex", str)));
      */
  return chessgame_make(allmoves, step);
}

static char *
complex_to_str(const Complex *c)
{
  char *result = psprintf("(%.*g, %.*g)",
    DBL_DIG, c->a,
    DBL_DIG, c->b);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(complex_in);
Datum
complex_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_COMPLEX_P(complex_parse(str));
}

PG_FUNCTION_INFO_V1(complex_out);
Datum
complex_out(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  char *result = complex_to_str(c);
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(complex_recv);
Datum
complex_recv(PG_FUNCTION_ARGS)
{
  StringInfo  buf = (StringInfo) PG_GETARG_POINTER(0);
  Complex *c = (Complex *) palloc(sizeof(Complex));
  c->a = pq_getmsgfloat8(buf);
  c->b = pq_getmsgfloat8(buf);
  PG_RETURN_COMPLEX_P(c);
}

PG_FUNCTION_INFO_V1(complex_send);
Datum
complex_send(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendfloat8(&buf, c->a);
  pq_sendfloat8(&buf, c->b);
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(complex_cast_from_text);
Datum
complex_cast_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = DatumGetCString(DirectFunctionCall1(textout,
               PointerGetDatum(txt)));
  PG_RETURN_COMPLEX_P(complex_parse(str));
}

PG_FUNCTION_INFO_V1(complex_cast_to_text);
Datum
complex_cast_to_text(PG_FUNCTION_ARGS)
{
  Complex *c  = PG_GETARG_COMPLEX_P(0);
  text *out = (text *)DirectFunctionCall1(textin,
            PointerGetDatum(complex_to_str(c)));
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_TEXT_P(out);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(complex_constructor);
Datum
complex_constructor(PG_FUNCTION_ARGS)
{
  double a = PG_GETARG_FLOAT8(0);
  double b = PG_GETARG_FLOAT8(1);
  PG_RETURN_COMPLEX_P(complex_make(a, b));
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(complex_re);
Datum
complex_re(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  double result = c->a;
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(complex_im);
Datum
complex_im(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  double result = c->b;
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(complex_conj);
Datum
complex_conj(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *result = complex_make(c->a, -c->b);
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_COMPLEX_P(result);
}

/*****************************************************************************/

static bool
complex_eq_internal(Complex *c, Complex *d)
{
  return FPeq(c->a, d->a) && FPeq(c->b, d->b);
}

PG_FUNCTION_INFO_V1(complex_eq);
Datum
complex_eq(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = complex_eq_internal(c, d);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_ne);
Datum
complex_ne(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = !complex_eq_internal(c, d);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_left);
Datum
complex_left(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = FPlt(c->a, d->a);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_right);
Datum
complex_right(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = FPgt(c->a, d->a);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_below);
Datum
complex_below(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = FPlt(c->b, d->b);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(complex_above);
Datum
complex_above(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  bool result = FPgt(c->b, d->b);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(complex_add);
Datum
complex_add(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  Complex *result = complex_make(c->a + d->a, c->b + d->b);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_COMPLEX_P(result);
}

PG_FUNCTION_INFO_V1(complex_sub);
Datum
complex_sub(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  Complex *result = complex_make(c->a - d->a, c->b - d->b);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_COMPLEX_P(result);
}

PG_FUNCTION_INFO_V1(complex_mul);
Datum
complex_mul(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  Complex *result = complex_make(
    c->a*d->a - c->b*d->b, 
    c->b*d->a + c->a*d->b);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_COMPLEX_P(result);
}

PG_FUNCTION_INFO_V1(complex_div);
Datum
complex_div(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  double div;
  Complex *result;
  if (FPzero(d->a) && FPzero(d->b))
  {
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Can only divide by a non-zero complex number")));
  }
  div = (d->a * d->a) + (d->b * d->b);
  result = complex_make(
    (c->a*d->a + c->b*d->b) / div, 
    (c->b*d->a - c->a*d->b) / div);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_COMPLEX_P(result);
}

/*****************************************************************************/

static double
complex_dist_internal(Complex *c, Complex *d)
{
  double x = fabs(c->a - d->a);
  double y = fabs(c->b - d->b);
  double yx, result;
  if (x < y)
  {
    double temp = x;
    x = y;
    y = temp;
  }
  if (FPzero(y))
    return x;
  yx = y / x;
  result = sqrt(1.0 + (yx * yx));
  return result;
}

PG_FUNCTION_INFO_V1(complex_dist);
Datum
complex_dist(PG_FUNCTION_ARGS)
{
  Complex *c = PG_GETARG_COMPLEX_P(0);
  Complex *d = PG_GETARG_COMPLEX_P(1);
  double result = complex_dist_internal(c, d);
  PG_FREE_IF_COPY(c, 0);
  PG_FREE_IF_COPY(d, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/