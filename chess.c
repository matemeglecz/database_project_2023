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



/*char *strndup(char *str, int chars)
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
}*/

/*****************************************************************************/

PG_MODULE_MAGIC;
static Chessgame *
chessgame_make(char* game)
{
  Chessgame *c = (Chessgame*)palloc(sizeof(Chessgame));

  // copy the string to the game
  //c->game = (char*)palloc(sizeof(char) * strlen(game));
  strcpy(c->game, game);
  

  if(c->game == NULL)
  {
   ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("invalid input syntax for type %s: \"%s\"", "chessgame", game)));
  }


  return c;
}

/*****************************************************************************/

static char*
sanmove_to_str(const SANmove* s)
{
    // allocate a 30 char array for the result
    char* result = (char*)palloc(sizeof(char) * 30);
    result[0] = '\0';
    
    if(s->piece != '0' && s->piece != 'p')
    {
        int len = strlen(result);
        result[len] = s->piece;
        result[len+1] = '\0';
    }
    if(s->from_file != '0')
    {
        //strcat(result, &s->from_file);
        int len = strlen(result);
        result[len] = s->from_file;
        result[len+1] = '\0';
    }
    if(s->from_rank != 0)
    {
        char rank[2];
        sprintf(rank, "%d", s->from_rank);
        strcat(result, rank);        
    }
    if(s->capture)
    {
        strcat(result, "x");
    }
    if(s->file != '0')
    {
        //strcat(result, &s->file);
        int len = strlen(result);
        result[len] = s->file;
        result[len+1] = '\0';
    }
    if(s->rank != 0)
    {
        char rank[2];
        sprintf(rank,"%d", s->rank);
        strcat(result, rank);        
    }
    if(s->promotion)
    {
        strcat(result, "=");
    }
    if(s->check)
    {
        strcat(result, "+");
    }
    if(s->checkmate)
    {
        strcat(result, "#");
    }
    if(s->drawoffer)
    {
        strcat(result, "=");
    }
    if(s->castle)
    {
        strcat(result, "0-0");
    }

    return result;
}

static char *
chessgamehelper_to_str(const Chessgame_helper *c)
{
    // create string from c moves
    char* result = (char*)palloc(sizeof(char) * 256);
    result[0] = '\0';
    
    for (int i = 0; i < c->size; ++i)
    {
        char* move = (char*)palloc(sizeof(char) * 10);
        sprintf(move,"%d. ", i+1);
        strcat(result, move);
        strcat(result, sanmove_to_str(&c->moves[i][0]));
        strcat(result, " ");
        if(c->moves[i][1].piece != '0'){
          strcat(result, sanmove_to_str(&c->moves[i][1]));
          strcat(result, " ");
        }

    }

    return result;
}

static char *
chessgame_to_str(const Chessgame *c)
{
  char *result = psprintf("%s", c->game);
  return result;
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
    char *str = (char*)palloc(sizeof(in));
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

static Chessgame_helper *
chessgame_helper_parse(char in[])
{
  //remove line breaks and copy the input string to a new string
  char *str = (char*)palloc(sizeof(char) * strlen(in));
  int len = strlen(in);
  int j = 0;
  for (int i = 0; i < len; ++i)
  {
      if (in[i] != '\n' && in[i] != '\r' && in[i] != '\t')
      {
          str[j] = in[i];
          j++;
      }     
  }

  str[j] = '\0';

  //remove line breaks
  //str = strtok(str, "\n");

  // steps in game
  int step = 0;
  //create a 2 dim dynamic array for storing all the move pairs 2 by 100
  SANmove **allmoves = (SANmove **)palloc(sizeof(SANmove*) * 100);
  for (int i = 0; i < 100; ++i)
  {
      allmoves[i] = (SANmove *)palloc(2 * sizeof(SANmove));

  }


  //iterate the str until a '.' is found, and process the string between the two dots, the this for the whole string
 
  char *dot = strchr(str, '.');
  if (dot != NULL)
  {
    char *dot2 = strchr(dot + 1, '.');
    bool end_reached = false;
    while (dot != NULL && !end_reached)
    {
      // create an array to store 2 sanmoves
      SANmove sanmoves[2];
      // initalize sanmives with empty sanmoves
      sanmoves[0] = sanmove_parse("");
      sanmoves[1] = sanmove_parse("");

      // if dot2 and dot is the same than dot2 is the end of the string
      if (dot2 == NULL)
      {
        dot2 = strchr(dot + 1, '\0');
          end_reached = true;
      }
      //char san[dot2 - dot];
      char san[64];
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
  
  Chessgame_helper *c_helper = (Chessgame_helper*)palloc(sizeof(Chessgame_helper));
  c_helper->moves = allmoves;
  c_helper->size = step;
  return c_helper;
}

static Chessgame *
chessgame_parse(char in[])
{
/*
  if (sscanf(str, " ( %lf , %lf )", &a, &b) != 2)
    ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("invalid input syntax for type %s: \"%s\"", "complex", str)));
      */

  Chessgame_helper* c_helper = chessgame_helper_parse(in);
  

  char* game_str = chessgamehelper_to_str(c_helper);

  

  return chessgame_make(game_str);
}


/*****************************************************************************/

PG_FUNCTION_INFO_V1(chessgame_in);
Datum
chessgame_in(PG_FUNCTION_ARGS)
{
  char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_CHESSGAME_P(chessgame_parse(str));
}

PG_FUNCTION_INFO_V1(chessgame_out);
Datum
chessgame_out(PG_FUNCTION_ARGS)
{
  Chessgame *c = PG_GETARG_CHESSGAME_P(0);
  //char* result = c->game;
  char *result = palloc(sizeof(char) * strlen(c->game));
  strcpy(result, c->game);
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(chessgame_recv);
Datum
chessgame_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Chessgame *c = (Chessgame *) palloc(sizeof(Chessgame));
  char* game = pq_getmsgstring(buf);
  strcpy(c->game, game);
  PG_RETURN_CHESSGAME_P(c);
}

PG_FUNCTION_INFO_V1(chessgame_send);
Datum
chessgame_send(PG_FUNCTION_ARGS)
{
  Chessgame *c = PG_GETARG_CHESSGAME_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendstring(&buf, c->game);
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(chessgame_cast_from_text);
Datum
chessgame_cast_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = DatumGetCString(DirectFunctionCall1(textout,
               PointerGetDatum(txt)));
  PG_RETURN_CHESSGAME_P(chessgame_parse(str));
}

PG_FUNCTION_INFO_V1(chessgame_cast_to_text);
Datum
chessgame_cast_to_text(PG_FUNCTION_ARGS)
{
  Chessgame *c  = PG_GETARG_CHESSGAME_P(0);
  
  text *out = (text *)DirectFunctionCall1(textin,
            PointerGetDatum(c->game));
  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_TEXT_P(out);
}


/*****************************************************************************/

PG_FUNCTION_INFO_V1(chessgame_constructor);
Datum
chessgame_constructor(PG_FUNCTION_ARGS)
{
  char* game = PG_GETARG_CSTRING(0);
  Chessgame *c = chessgame_parse(game);
  PG_RETURN_CHESSGAME_P(c);
}


/*****************************************************************************/

PG_FUNCTION_INFO_V1(getFirstMoves);
Datum
getFirstMoves(PG_FUNCTION_ARGS)
{
  Chessgame *c = PG_GETARG_CHESSGAME_P(0);
  int n = PG_GETARG_INT32(1);


  if (n <= 0)
    ereport(ERROR,(errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("invalid subscript value: %d", n)));
    
  // parse c->game into chessgame_helper
  Chessgame_helper* c_helper = chessgame_helper_parse(c->game);

  if (n > c_helper->size)
      n = c_helper->size * 2;

  float full_moves = ((float)n)/2;
  int needed_rounds = ceil(full_moves);

  // create a new chessgame_helper

  c_helper->size = needed_rounds;
  if(n % 2 == 1)
      c_helper->moves[needed_rounds-1][1].piece = '0';

  char* game_str = chessgamehelper_to_str(c_helper);

  // create a new chessgame
  Chessgame* result = chessgame_make(game_str);

  PG_FREE_IF_COPY(c, 0);
  PG_RETURN_CHESSGAME_P(result);
}
