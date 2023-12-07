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
#include "smallchesslib.h"
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

PG_FUNCTION_INFO_V1(hasOpening);
Datum 
hasOpening(PG_FUNCTION_ARGS)
{
    Chessgame *c1 = PG_GETARG_CHESSGAME_P(0);
    Chessgame *c2 = PG_GETARG_CHESSGAME_P(1);
    // parse c1->game into chessgame_helper
    Chessgame_helper* c1_helper = chessgame_helper_parse(c1->game);
    // parse c2->game into chessgame_helper
    Chessgame_helper* c2_helper = chessgame_helper_parse(c2->game);
    bool result = true;
    // iterate through c2_helper->moves
    for(int i=0; i<c2_helper->size;i++)
    {
        if(strcmp(sanmove_to_str(&c1_helper->moves[i][0]), sanmove_to_str(&c2_helper->moves[i][0])) != 0)
            result = false;
            break;
        if(c2_helper->moves[i][1].piece == '0')
            break;
        if(strcmp(sanmove_to_str(&c1_helper->moves[i][1]), sanmove_to_str(&c2_helper->moves[i][1])) != 0)
            result = false;
            break;
    }
    

    PG_FREE_IF_COPY(c1, 0);
    PG_FREE_IF_COPY(c2, 1);
    PG_RETURN_BOOL(result);
}


/*****************************************************************************/

static Chessboard *
chessboard_make(char *fen) {

    Chessboard *result = palloc(sizeof(Chessboard));
    // int rank = 7;
    // int file = 0;
    int index = 0;
    if(result == NULL)
    {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("1 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
      // Parse the piece positions
    while (*fen && *fen != ' ') {
        if (*fen == '/') {
            fen++;
        } else if (*fen >= '1' && *fen <= '8') {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; i++) {
                result->board[index] = '0';
                index++;
            }
            fen++;
        } else {
            // Piece
            result->board[index] = *fen;
            index++;
            fen++;
        }
    }

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        // FEN is incomplete or malformed
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("2 invalid input syntax for type %s:  \"%s\"", "chessboard", fen)));
    }

    // Current color
    fen++;
    result->currentColor = *fen;

    // Move to the next part of FEN
    fen = strchr(fen, ' ');

    if (!fen) {
       ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("3 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Castling availability

    fen++;
    size_t castlingLength = strcspn(fen, " ");
    if (castlingLength > sizeof(result->castling) ) {
        castlingLength = sizeof(result->castling);
    }
    strncpy(result->castling, fen, castlingLength);

    result->castling[castlingLength] = '\0';
    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("6 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
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
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("4 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Half-move clock
    fen++;
    
    result->halfMoveClock = atoi(fen);

    // Move to the next part of FEN
    fen = strchr(fen, ' ');
    if (!fen) {
       ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("5 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }

    // Full-move number
    fen++;
    result->fullMoveNumber = atoi(fen);

    return result;
    
}

/*****************************************************************************/

static Chessboard * chessboard_parse(char *fen){
    if(!fen){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    errmsg("parse invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
    
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
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(0);
    char *result = palloc(sizeof(char) * strlen(chessboard_to_str(chessboard)));
    strcpy(result, chessboard_to_str(chessboard));
    PG_FREE_IF_COPY(chessboard, 0);
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(chessboard_send);
Datum chessboard_send(PG_FUNCTION_ARGS) {
    StringInfoData buf;
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(0);

    pq_begintypsend(&buf);

    pq_sendstring(&buf, chessboard->board);

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
    result = chessboard_parse(pq_getmsgstring(buf));
    // memcpy(result->board, pq_getmsgstring(buf, sizeof(result->board)), sizeof(result->board));

    // result->currentColor = pq_getmsgbyte(buf);

    // strncpy(result->castling, pq_getmsgstring(buf), sizeof(result->castling));
    // strncpy(result->enPassant, pq_getmsgstring(buf), sizeof(result->enPassant));

    // result->halfMoveClock = pq_getmsgint(buf, sizeof(result->halfMoveClock));
    // result->fullMoveNumber = pq_getmsgint(buf, sizeof(result->fullMoveNumber));

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
    if(!fen){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    errmsg("constructor invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }
    
    Chessboard *result = chessboard_parse(fen);
    PG_RETURN_CHESSBOARD_P(result);
}
/*****************************************************************************/

PG_FUNCTION_INFO_V1(getBoard);
Datum getBoard(PG_FUNCTION_ARGS) {
    // Check and extract function arguments
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    // Get the chess game and half-move count parameters
    Chessgame *chessgame = PG_GETARG_CHESSGAME_P(0);
    int32 half_moves = PG_GETARG_INT32(1);

    // Convert text input to C string
    char *chessgame_str = chessgame_to_str(chessgame);

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

PG_FUNCTION_INFO_V1(hasBoard);
Datum hasBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    Chessgame *chessgame = PG_GETARG_CHESSGAME_P(0);
    Chessboard *chessboard = PG_GETARG_CHESSBOARD_P(1);
    int32 half_moves = PG_GETARG_INT32(2);

    // Convert text input to C string
    char *chessgame_text = chessgame_to_str(chessgame);
     char *chessboard_text = chessboard_to_str(chessboard);

    if (half_moves < 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    }

    char *chessgame_str = text_to_cstring(chessgame_text);
    char *chessboard_str = text_to_cstring(chessboard_text);

    // Check if the chess game contains the given board state in its initial N half-moves
    bool contains_board_state = smallchesslib_contains_board_state(chessgame_str, chessboard_str, half_moves);

    PG_RETURN_BOOL(contains_board_state);
}
