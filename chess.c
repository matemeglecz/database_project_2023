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

#define nullptr ((void*)0)

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

// Convertit un indice 2D en indice 1D
int index2DTo1D(int row, int col) {
    return row * 8 + col;
}

static Chessboard *
chessboard_make(char *fen) {

    Chessboard *result = palloc(sizeof(Chessboard));
    char board2D[8][8];
    int rank = 7;
    int file = 0;
    int index = 63;
    /*if(result == NULL)
    {
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("1 invalid input syntax for type %s: \"%s\"", "chessboard", fen)));
    }*/
      // Parse the piece positions
    while (*fen && *fen != ' ') {
        if (*fen == '/') {
            fen++;
            rank --;
            file =0;
        } else if (*fen >= '1' && *fen <= '8') {
            // Empty squares
            int count = *fen - '0';
            for (int i = 0; i < count; i++) {
                board2D[rank][file] = '0';
                index++;
                file++;
            }
            fen++;
        } else {
            // Piece
            board2D[rank][file] = *fen;
            index++;
            fen++;
            file++;
        }
    }

    for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      int index = index2DTo1D(row, col);
      result->board[index] = board2D[row][col];
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


void index1DTo2D(int index, int *row, int *col) {
    *row = index / 8;
    *col = index % 8;
}

static bool chessboard_update(Chessboard* board, const SANmove* s, char constPlayer){
  // Convertit le tableau 1D en tableau 2D
    char board2D[8][8];
    for (int i = 0; i < 64; ++i) {
        int row, col;
        index1DTo2D(i, &row, &col);
        board2D[row][col] = board->board[i];
    }
    int file = s->file - 'a';
    bool move = false;
    
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
          elog(DEBUG1, "%c ", board2D[i][j]);
      }
      elog(DEBUG1, "\r\n");
    }
    
    char piece_to_find = s->piece;
    if(constPlayer == 'w' && s->piece == 'p'){
      piece_to_find = toupper(s->piece);
    }

    if(constPlayer == 'b'){
      piece_to_find = tolower(s->piece);
    }
    /*if(true){
      ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      errmsg("invalid move %c: file : %i rank: %i",  board2D[0][0], file, s->rank)));
    }*/
    
    for(int i = 0; i <8; i++){
      for(int j =0; j<8;j++){
        if(board2D[i][j] == piece_to_find){
          int possibleMove[64][2];
          int counterPossibleMove = 0;

          bool ss = true;
          bool se = true;
          bool e = true;
          bool ne = true;
          bool n = true;
          bool nw = true;
          bool w = true;
          bool sw = true;

          if(piece_to_find == 'p'){
            if(i-1>=0){
              if(board2D[i-1][j] == '0'){
                //rank
                possibleMove[counterPossibleMove][0] = i-1;
                //file
                possibleMove[counterPossibleMove][1] = j;
                counterPossibleMove++;
              }
              
            }
            
            if(s->capture){
              //rank
              possibleMove[counterPossibleMove][0] = i-1;
              //file
              possibleMove[counterPossibleMove][1] = j-1;
              counterPossibleMove++;
              //rank
              possibleMove[counterPossibleMove][0] = i-1;
              //file
              possibleMove[counterPossibleMove][1] = j+1;
              counterPossibleMove++;
            }
            
            
            if(i == 6){
              //rank
              possibleMove[counterPossibleMove][0] = i-2;
              //file
              possibleMove[counterPossibleMove][1] = j;
              counterPossibleMove++;
            }
            
            for(int itera = 0; itera < counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
                
              }
            }


          }

          if(piece_to_find == 'P'){
            if(i+1<8){
              if(board2D[i+1][j] == '0'){
                //rank
                possibleMove[counterPossibleMove][0] = i+1;
                //file
                possibleMove[counterPossibleMove][1] = j;
                counterPossibleMove++;
              }
            }
            
            if(s->capture){
              //rank
              possibleMove[counterPossibleMove][0] = i+1;
              //file
              possibleMove[counterPossibleMove][1] = j-1;
              counterPossibleMove++;
              //rank
              possibleMove[counterPossibleMove][0] = i+1;
              //file
              possibleMove[counterPossibleMove][1] = j+1;
              counterPossibleMove++;
            }
            
            
            if(i == 1){
              //rank
              possibleMove[counterPossibleMove][0] = i+2;
              //file
              possibleMove[counterPossibleMove][1] = j;
              counterPossibleMove++;
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
              }
            }

          }

          if(piece_to_find == 'r' || piece_to_find == 'R'){
            for(int delta = 1; delta < 8; delta++){
              
              if(j+delta < 8){
                if(constPlayer == 'b' && ( board2D[i][j+delta] == 'b' || board2D[i][j+delta] == 'n' || board2D[i][j+delta] == 'q' || board2D[i][j+delta] == 'k' || board2D[i][j+delta] == 'p')){
                  ss = false;
                }

                if(constPlayer == 'w' && ( board2D[i][j+delta] == 'B' || board2D[i][j+delta] == 'N' || board2D[i][j+delta] == 'Q' || board2D[i][j+delta] == 'K' || board2D[i][j+delta] == 'P')){
                  ss = false;
                }
                if(ss){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }
              if(j - delta >= 0){
                if(constPlayer == 'b' && (board2D[i][j-delta] == 'b' || board2D[i][j-delta] == 'n' || board2D[i][j-delta] == 'q' || board2D[i][j-delta] == 'k' || board2D[i][j-delta] == 'p')){
                  e = false;
                }

                 if(constPlayer == 'w' && (board2D[i][j-delta] == 'B' || board2D[i][j-delta] == 'N' || board2D[i][j-delta] == 'Q' || board2D[i][j-delta] == 'K' || board2D[i][j-delta] == 'P')){
                   e = false;
                }
                if(e){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  possibleMove[counterPossibleMove][1] = j - delta;
                  counterPossibleMove++;
                }
                
              }

              if(i+delta < 8){
               if(constPlayer == 'b' && ( board2D[i+delta][j] == 'b' || board2D[i+delta][j] == 'n' || board2D[i+delta][j] == 'q' || board2D[i+delta][j] == 'k' || board2D[i+delta][j] == 'p')){
                  n = false;
                }

                if(constPlayer == 'w' && (board2D[i+delta][j] == 'B' || board2D[i+delta][j] == 'N' || board2D[i+delta][j] == 'Q' || board2D[i+delta][j] == 'K' || board2D[i+delta][j] == 'P')){
                 n = false;

                }
                if(n){
                  //rank
                  possibleMove[counterPossibleMove][0] = i+delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j;
                  counterPossibleMove++;
                }
                
              }
              if(i - delta >= 0){
                if(constPlayer == 'b' && (board2D[i - delta][j] == 'b' || board2D[i - delta][j] == 'n' || board2D[i - delta][j] == 'q' || board2D[i - delta][j] == 'k' || board2D[i - delta][j] == 'p')){
                  w = false;
                }

                 if(constPlayer == 'w' && ( board2D[i - delta][j] == 'B' || board2D[i - delta][j] == 'N' || board2D[i - delta][j]== 'Q' || board2D[i - delta][j] == 'K' || board2D[i - delta][j] == 'P')){
                   w = false;

                }
                if(w){
                   //rank
                  possibleMove[counterPossibleMove][0] = i - delta;
                  possibleMove[counterPossibleMove][1] = j;
                  counterPossibleMove++;
                }
               
              }
              
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
              }
            }
          }

          if(piece_to_find == 'n' || piece_to_find == 'N'){
            if(i + 2 < 8){
              if(j - 1 >= 0){
                //rank
                possibleMove[counterPossibleMove][0] = i+2;
                //file
                possibleMove[counterPossibleMove][1] = j - 1;
                counterPossibleMove++;
              }
              if(j + 1 < 8){
                //rank
                possibleMove[counterPossibleMove][0] = i+2;
                //file
                possibleMove[counterPossibleMove][1] = j + 1;
                counterPossibleMove++;
              }
              
            }

            if(i - 2 >= 0){
              if(j - 1 >= 0){
                //rank
                possibleMove[counterPossibleMove][0] = i - 2;
                //file
                possibleMove[counterPossibleMove][1] = j - 1;
                counterPossibleMove++;
              }
              if(j + 1 < 8){
                //rank
                possibleMove[counterPossibleMove][0] = i - 2;
                //file
                possibleMove[counterPossibleMove][1] = j + 1;
                counterPossibleMove++;
              }
              
            }

             if(j - 2 >= 0){
              if(i - 1 >= 0){
                //rank
                possibleMove[counterPossibleMove][0] = i - 1;
                //file
                possibleMove[counterPossibleMove][1] = j - 2;
                counterPossibleMove++;
              }
              if(i + 1 < 8){
                //rank
                possibleMove[counterPossibleMove][0] = i + 1;
                //file
                possibleMove[counterPossibleMove][1] = j - 2;
                counterPossibleMove++;
              }
              
            }

             if(j + 2 < 8){
              if(i - 1 >= 0){
                //rank
                possibleMove[counterPossibleMove][0] = i - 1;
                //file
                possibleMove[counterPossibleMove][1] = j + 2;
                counterPossibleMove++;
              }
              if(i + 1 < 8){
                //rank
                possibleMove[counterPossibleMove][0] = i + 1;
                //file
                possibleMove[counterPossibleMove][1] = j + 2;
                counterPossibleMove++;
              }
              
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;

                    move = true;
                  }
                }else{
                  
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  // if(piece_to_find == 'N'){
                  //   /*char* result = (char*)malloc(sizeof(char) * 65);  
                  //   int index = 0;

                  //   for (int i = 0; i < 8; i++) {
                  //       for (int j = 0; j < 8; j++) {
                  //           result[index++] = board2D[i][j];
                  //       }
                  //   }
                  //   char * sanStr = (char*)malloc(sizeof(char) * 4);
                  //   sanStr = sanmove_to_str(&s);
                  //   result[index] = '\0'; 
                  //   if(index != 0){
                  //     ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                  //     errmsg("test board at move : %s , array : %s",  sanStr, result)));
                  //   } */
                  //   /*ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                  //   errmsg("invalid move board2d %c  board1d %c: file : %i rank: %i, row %i , col %i",  board2D[0][6], board->board[6], file, s->rank, i, j)));
                  //  */
                  // }

                  move = true;
                }
              }
            }

          }

          if(piece_to_find == 'b' || piece_to_find == 'B'){
            for(int delta = 1 ; delta < 8; delta ++){
              if(i - delta >= 0 && j - delta >= 0){
                if(constPlayer == 'b' && (board2D[i - delta][j - delta] == 'r' || board2D[i - delta][j - delta] == 'n' || board2D[i - delta][j - delta] == 'q' || board2D[i - delta][j - delta] == 'k' || board2D[i - delta][j - delta] == 'p')){
                  se = false;
                }

                if(constPlayer == 'w' && (board2D[i - delta][j - delta] == 'R' || board2D[i - delta][j - delta] == 'N' || board2D[i - delta][j - delta] == 'Q' || board2D[i - delta][j - delta] == 'K' || board2D[i - delta][j - delta] == 'P')){
                  se = false;

                }
                if(se){
                  //rank
                  possibleMove[counterPossibleMove][0] = i - delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j - delta;
                  counterPossibleMove++;
                }
                
              }

              if(i - delta >= 0 && j + delta < 8){
                 if(constPlayer == 'b' && (board2D[i - delta][j + delta] == 'r' || board2D[i - delta][j + delta] == 'n' || board2D[i - delta][j + delta] == 'q' || board2D[i - delta][j + delta] == 'k' || board2D[i - delta][j + delta] == 'p')){
                  ne = false;
                }

                if(constPlayer == 'w' && (board2D[i - delta][j + delta] == 'R' || board2D[i - delta][j + delta] == 'N' || board2D[i - delta][j + delta] == 'Q' || board2D[i - delta][j + delta] == 'K' || board2D[i - delta][j + delta] == 'P')){
                  ne = false;
                }

                if(ne){
                  //rank
                  possibleMove[counterPossibleMove][0] = i - delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }

              if(i + delta < 8 && j - delta >= 0){
                if(constPlayer == 'b' && (board2D[i + delta][j - delta] == 'r' || board2D[i + delta][j - delta] == 'n' || board2D[i + delta][j - delta] == 'q' || board2D[i + delta][j - delta] == 'k' || board2D[i + delta][j - delta] == 'p')){
                  sw = false;
                }

                if(constPlayer == 'w' && (board2D[i + delta][j - delta] == 'R' || board2D[i + delta][j - delta] == 'N' || board2D[i + delta][j - delta] == 'Q' || board2D[i + delta][j - delta] == 'K' || board2D[i + delta][j - delta] == 'P')){
                  sw = false;
                }
                if(sw){
                  //rank
                  possibleMove[counterPossibleMove][0] = i + delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j - delta;
                  counterPossibleMove++;
                }
                
              }

              if(i + delta < 8 && j + delta < 8){
                if(constPlayer == 'b' && (board2D[i + delta][j + delta] == 'r' ||  board2D[i + delta][j + delta] == 'n' || board2D[i + delta][j + delta] == 'q' || board2D[i + delta][j + delta] == 'k' || board2D[i + delta][j + delta] == 'p')){
                  nw = false;
                }

                if(constPlayer == 'w' && (board2D[i + delta][j + delta] == 'R' ||  board2D[i + delta][j + delta] == 'N' || board2D[i + delta][j + delta] == 'Q' || board2D[i + delta][j + delta] == 'K' || board2D[i + delta][j + delta] == 'P')){
                  nw = false;

                }

                if(nw){
                  //rank
                  possibleMove[counterPossibleMove][0] = i + delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
              }
            }
          }

         
         
          if(piece_to_find == 'q' ||piece_to_find == 'Q'){
            for(int delta = 1; delta < 8; delta++){
              
             if(j+delta < 8){
                if(constPlayer == 'b' && (board2D[i][j+delta] == 'r' || board2D[i][j+delta] == 'b' || board2D[i][j+delta] == 'n' || board2D[i][j+delta] == 'k' || board2D[i][j+delta] == 'p')){
                  ss = false;
                }

                if(constPlayer == 'w' && (board2D[i][j+delta]== 'R' || board2D[i][j+delta] == 'B' || board2D[i][j+delta]== 'N' || board2D[i][j+delta] == 'K' || board2D[i][j+delta] == 'P')){
                  ss = false;
                }
                if(ss){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }
              if(j - delta >= 0){
                if(constPlayer == 'b' && (board2D[i][j-delta] == 'r' || board2D[i][j-delta] == 'b' || board2D[i][j-delta] == 'n'  || board2D[i][j-delta] == 'k' || board2D[i][j-delta] == 'p')){
                  se = false;
                }

                if(constPlayer == 'w' && (board2D[i][j-delta] == 'R' || board2D[i][j-delta] == 'B' || board2D[i][j-delta] == 'N'  || board2D[i][j-delta]== 'K' || board2D[i][j-delta] == 'P')){
                  se = false;
                }
                if(se){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  possibleMove[counterPossibleMove][1] = j - delta;
                  counterPossibleMove++;
                }
                
              }

              if(i+delta < 8){
                if(constPlayer == 'b' && (board2D[i+delta][j] == 'r' || board2D[i+delta][j] == 'b' || board2D[i+delta][j] == 'n' || board2D[i+delta][j] == 'k' || board2D[i+delta][j] == 'p')){
                  e = false;
                }

                if(constPlayer == 'w' && (board2D[i+delta][j] == 'R' || board2D[i+delta][j] == 'B' || board2D[i+delta][j] == 'N'  || board2D[i+delta][j] == 'K' || board2D[i+delta][j] == 'P')){
                  e = false;
                }
                if(e){
                  //rank
                  possibleMove[counterPossibleMove][0] = i+delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j;
                  counterPossibleMove++;
                }
                
              }
              if(i - delta >= 0){
               if(constPlayer == 'b' && (board2D[i - delta][j] == 'r' || board2D[i - delta][j] == 'b' || board2D[i - delta][j] == 'n' || board2D[i - delta][j] == 'k' || board2D[i - delta][j] == 'p')){
                  ne = false;
                }

                 if(constPlayer == 'w' && (board2D[i - delta][j] == 'R' || board2D[i - delta][j] == 'B' || board2D[i - delta][j] == 'N' || board2D[i - delta][j] == 'K' || board2D[i - delta][j] == 'P')){
                  ne = false;
                }
                if(ne){
                    //rank
                    possibleMove[counterPossibleMove][0] = i - delta;
                    possibleMove[counterPossibleMove][1] = j;
                    counterPossibleMove++;
                }
                
              }
              
            }
            for(int delta = 1 ; delta < 8; delta ++){
              if(i - delta >= 0 && j - delta >= 0){
                if(constPlayer == 'b' && (board2D[i - delta][j - delta] == 'r' || board2D[i - delta][j - delta] == 'b' || board2D[i - delta][j - delta] == 'n' || board2D[i - delta][j - delta] == 'k' || board2D[i - delta][j - delta] == 'p')){
                  n = false;
                }

                if(constPlayer == 'w' && (board2D[i - delta][j - delta] == 'R' || board2D[i - delta][j - delta] == 'B' || board2D[i - delta][j - delta] == 'N' || board2D[i - delta][j - delta] == 'K' || board2D[i - delta][j - delta] == 'P')){
                  n = false;
                }

                if(n){
                  //rank
                  possibleMove[counterPossibleMove][0] = i - delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j - delta;
                  counterPossibleMove++;
                }
                
              }

              if(i - delta >= 0 && j + delta < 8){
               if(constPlayer == 'b' && (board2D[i - delta][j + delta] == 'r' || board2D[i - delta][j + delta] == 'b' || board2D[i - delta][j + delta] == 'n'  || board2D[i - delta][j + delta] == 'k' || board2D[i - delta][j + delta] == 'p')){
                  nw = false;
                }

                if(constPlayer == 'w' && (board2D[i - delta][j + delta] == 'R' || board2D[i - delta][j + delta] == 'B' || board2D[i - delta][j + delta] == 'N' || board2D[i - delta][j + delta] == 'K' || board2D[i - delta][j + delta] == 'P')){
                  nw = false;
                }
                
                if(nw){
                  //rank
                  possibleMove[counterPossibleMove][0] = i - delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }

              if(i + delta < 8 && j - delta >= 0){
               if(constPlayer == 'b' && (board2D[i + delta][j - delta] == 'r' || board2D[i + delta][j - delta] == 'b' || board2D[i + delta][j - delta] == 'n'  || board2D[i + delta][j - delta] == 'k' || board2D[i + delta][j - delta] == 'p')){
                  w = false;
                }

                if(constPlayer == 'w' && (board2D[i + delta][j - delta] == 'R' || board2D[i + delta][j - delta] == 'B' || board2D[i + delta][j - delta] == 'N'  || board2D[i + delta][j - delta] == 'K' || board2D[i + delta][j - delta]== 'P')){
                  w = false;

                }
                if(w){
                    //rank
                    possibleMove[counterPossibleMove][0] = i + delta;
                    //file
                    possibleMove[counterPossibleMove][1] = j - delta;
                    counterPossibleMove++;
                }
                
              }

              if(i + delta < 8 && j + delta < 8){
               if(constPlayer == 'b' && (board2D[i + delta][j + delta] == 'r' || board2D[i + delta][j + delta] == 'b' || board2D[i + delta][j + delta] == 'n'  || board2D[i + delta][j + delta] == 'k' || board2D[i + delta][j + delta] == 'p')){
                  sw = false;
                }

                 if(constPlayer == 'w' && (board2D[i + delta][j + delta] == 'R' || board2D[i + delta][j + delta] == 'B' || board2D[i + delta][j + delta] == 'N'  || board2D[i + delta][j + delta] == 'K' || board2D[i + delta][j + delta] == 'P')){
                  sw = false;

                }
                //rank
                if(sw){
                  possibleMove[counterPossibleMove][0] = i + delta;
                  //file
                  possibleMove[counterPossibleMove][1] = j + delta;
                  counterPossibleMove++;
                }
                
              }
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
               if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
              }
            }
          }

          if(piece_to_find == 'k' || piece_to_find == 'K'){
            if(i + 1 < 8){
                if(constPlayer == 'b' && (board2D[i + 1][j] == 'r' || board2D[i + 1][j] == 'b' || board2D[i + 1][j] == 'n' || board2D[i + 1][j] == 'q' || board2D[i + 1][j] == 'k' || board2D[i + 1][j] == 'p')){
                  se = false;
                }

                 if(constPlayer == 'w' && (board2D[i + 1][j] == 'R' || board2D[i + 1][j] == 'B' || board2D[i + 1][j] == 'N' || board2D[i + 1][j] == 'Q' || board2D[i + 1][j] == 'K' || board2D[i + 1][j] == 'P')){
                  se = false;
                }

                if(se){
                  //rank
                  possibleMove[counterPossibleMove][0] = i + 1;
                  //file
                  possibleMove[counterPossibleMove][1] = j;
                  counterPossibleMove++;
                }
                
                if(j + 1 < 8){
                  if(constPlayer == 'b' && (board2D[i + 1][j+1] == 'r' || board2D[i + 1][j+1] == 'b' || board2D[i + 1][j+1] == 'n' || board2D[i + 1][j+1] == 'q' || board2D[i + 1][j+1] == 'k' || board2D[i + 1][j+1] == 'p')){
                    e = false;
                  }

                  if(constPlayer == 'w' && (board2D[i + 1][j+1] == 'R' ||board2D[i + 1][j+1] == 'B' || board2D[i + 1][j+1]== 'N' || board2D[i + 1][j+1] == 'Q' || board2D[i + 1][j+1] == 'K' || board2D[i + 1][j+1]== 'P')){
                    e = false;

                  }
                  if(e){
                    //rank
                    possibleMove[counterPossibleMove][0] = i + 1;
                    //file
                    possibleMove[counterPossibleMove][1] = j + 1;
                    counterPossibleMove++;
                  }
                   
                }
                if(j - 1 >= 0){
                  if(constPlayer == 'b' && (board2D[i + 1][j-1] == 'r' || board2D[i + 1][j-1] == 'b' || board2D[i + 1][j-1] == 'n' || board2D[i + 1][j-1] == 'q' || board2D[i + 1][j-1] == 'k' || board2D[i + 1][j-1] == 'p')){
                    ne = false;
                  }

                  if(constPlayer == 'w' && (board2D[i + 1][j-1] == 'R' ||board2D[i + 1][j-1] == 'B' || board2D[i + 1][j-1]== 'N' || board2D[i + 1][j-1] == 'Q' || board2D[i + 1][j-1]== 'K' || board2D[i + 1][j-1] == 'P')){
                    ne = false;

                  }

                  if(ne){
                     //rank
                    possibleMove[counterPossibleMove][0] = i + 1;
                    //file
                    possibleMove[counterPossibleMove][1] = j - 1;
                    counterPossibleMove++;
                  }
                  
                }
            }
            if(i - 1 >= 0){
                if(constPlayer == 'b' && (board2D[i - 1][j] == 'r' || board2D[i - 1][j] == 'b' || board2D[i - 1][j] == 'n' || board2D[i - 1][j] == 'q' || board2D[i - 1][j] == 'k' || board2D[i - 1][j] == 'p')){
                  n =false;
                }

                if(constPlayer == 'w' && (board2D[i - 1][j] == 'R' ||board2D[i - 1][j] == 'B' || board2D[i - 1][j]== 'N' || board2D[i - 1][j] == 'Q' || board2D[i - 1][j]== 'K' || board2D[i - 1][j] == 'P')){
                    n =false;

                }
                if(n){
                  //rank
                  possibleMove[counterPossibleMove][0] = i - 1;
                  //file
                  possibleMove[counterPossibleMove][1] = j;
                  counterPossibleMove++;
                }
                

                 if(j + 1 < 8){
                  if(constPlayer == 'b' && (board2D[i - 1][j+1] == 'r' || board2D[i - 1][j+1] == 'b' || board2D[i - 1][j+1] == 'n' || board2D[i - 1][j+1] == 'q' || board2D[i - 1][j+1] == 'k' || board2D[i - 1][j+1] == 'p')){
                    nw = false;
                  }

                  if(constPlayer == 'w' && (board2D[i - 1][j+1] == 'R' ||board2D[i - 1][j+1] == 'B' || board2D[i - 1][j+1]== 'N' || board2D[i - 1][j+1] == 'Q' || board2D[i - 1][j+1]== 'K' || board2D[i - 1][j+1] == 'P')){
                    nw = false;

                  }

                  if(nw){
                     //rank
                    possibleMove[counterPossibleMove][0] = i - 1;
                    //file
                    possibleMove[counterPossibleMove][1] = j + 1;
                    counterPossibleMove++;
                  }
                  
                }
                if(j - 1 >= 0){
                 if(constPlayer == 'b' && (board2D[i - 1][j-1] == 'r' || board2D[i - 1][j-1] == 'b' || board2D[i - 1][j-1] == 'n' || board2D[i - 1][j-1] == 'q' || board2D[i - 1][j-1] == 'k' || board2D[i - 1][j-1] == 'p')){
                    w =false;
                  }

                  if(constPlayer == 'w' && (board2D[i - 1][j-1] == 'R' ||board2D[i - 1][j-1] == 'B' || board2D[i - 1][j-1]== 'N' || board2D[i - 1][j-1] == 'Q' || board2D[i - 1][j-1]== 'K' || board2D[i - 1][j-1]== 'P')){
                     w =false;
                  }
                  if(w){
                    //rank
                    possibleMove[counterPossibleMove][0] = i - 1;
                    //file
                    possibleMove[counterPossibleMove][1] = j - 1;
                    counterPossibleMove++;
                  }
                  
                }
            }

            if(j - 1 >= 0){
              if(constPlayer == 'b' && (board2D[i][j-1] == 'r' || board2D[i][j-1] == 'b' || board2D[i][j-1] == 'n' || board2D[i][j-1] == 'q' || board2D[i][j-1] == 'k' || board2D[i][j-1] == 'p')){
                  sw = false;
                }

                if(constPlayer == 'w' && (board2D[i][j-1] == 'R' ||board2D[i][j-1]== 'B' || board2D[i][j-1]== 'N' || board2D[i][j-1] == 'Q' || board2D[i][j-1]== 'K' || board2D[i][j-1]== 'P')){
                  sw = false;

                }
                if(sw){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  //file
                  possibleMove[counterPossibleMove][1] = j - 1;
                  counterPossibleMove++;
                }
              
            }

            if(j + 1 < 8){
               if(constPlayer == 'b' && (board2D[i][j+1] == 'r' || board2D[i][j+1] == 'b' || board2D[i][j+1] == 'n' || board2D[i][j+1] == 'q' || board2D[i][j+1] == 'k' || board2D[i][j+1] == 'p')){
                  ss = false;
                }

                if(constPlayer == 'w' && (board2D[i][j+1] == 'R' ||board2D[i][j+1]== 'B' || board2D[i][j+1]== 'N' || board2D[i][j+1] == 'Q' || board2D[i][j+1]== 'K' || board2D[i][j+1]== 'P')){
                  ss = false;

                }
                if(ss){
                  //rank
                  possibleMove[counterPossibleMove][0] = i;
                  //file
                  possibleMove[counterPossibleMove][1] = j + 1;
                  counterPossibleMove++;
                }
              
            }

            for(int itera = 0; itera< counterPossibleMove; itera++){
              if(file == possibleMove[itera][1] && (s->rank-1) == possibleMove[itera][0]){
                if(s->from_file != '0'){
                  int fromFile = s->from_file - 'a';
                  if(j == fromFile){
                    //possible move for this piece
                    board2D[i][j] = '0';
                    board2D[s->rank-1][file] = piece_to_find;
                    move = true;
                  }
                }else{
                  //possible move for this piece
                  board2D[i][j] = '0';
                  board2D[s->rank-1][file] = piece_to_find;
                  move = true;
                }
              }
            }
          }
        }
      }
    }

    
    

  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 8; ++col) {
      int index = index2DTo1D(row, col);
      board->board[index] = board2D[row][col];
      // if(piece_to_find == 'N' && index == 6){
      //               /*char* result = (char*)malloc(sizeof(char) * 65);  
      //               int index = 0;

      //               for (int i = 0; i < 8; i++) {
      //                   for (int j = 0; j < 8; j++) {
      //                       result[index++] = board2D[i][j];
      //                   }
      //               }
      //               char * sanStr = (char*)malloc(sizeof(char) * 4);
      //               sanStr = sanmove_to_str(&s);
      //               result[index] = '\0'; 
      //               if(index != 0){
      //                 ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      //                 errmsg("test board at move : %s , array : %s",  sanStr, result)));
      //               } */
      //               ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
      //               errmsg("invalid move board2d %c  board1d %c: file : %i rank: %i, row %i , col %i",  board2D[0][6], board->board[index], file, s->rank, row, col)));
      //             }
    }
  }

  return move;


}
  
Chessboard* getBoardPrivate(Chessgame* chessgame, int32 half_moves) {
    // Check and extract function arguments
    if (!chessgame) {
        return nullptr;
    }

    

    // Convert text input to C string
    //char *chessgame_str = chessgame_to_str(chessgame);
    Chessgame_helper* c_helper = chessgame_helper_parse(chessgame->game);
    Chessboard* boardState = chessboard_make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for(int i =0; i < half_moves;i++){
      //white move
      if(!chessboard_update(boardState ,&c_helper->moves[i][0], 'w')){
        return nullptr;
      }
      //black move
      if(!chessboard_update(boardState , &c_helper->moves[i][1], 'b')){
        return nullptr;
      }
    }

    // if (result != 0) {
    //     ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    // }

    // // Convert the FEN string to a PostgreSQL text type
    // text *fen_text = cstring_to_text(fen);

    // Return the FEN representation of the board state
   return boardState;
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
    //char *chessgame_str = chessgame_to_str(chessgame);
    Chessgame_helper* c_helper = chessgame_helper_parse(chessgame->game);
    Chessboard* boardState = chessboard_make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    for(int i =0; i < half_moves;i++){
      //white move
      if(!chessboard_update(boardState ,&c_helper->moves[i][0], 'w')){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("invalid move %s: \"%s\" piece : %c , rank : %i, file : %i, from rank : %i, from file : %i", "chessboard cannot be update",  sanmove_to_str(&c_helper->moves[i][0]), toupper(c_helper->moves[i][0].piece), c_helper->moves[i][0].rank, c_helper->moves[i][0].file - 'a', c_helper->moves[i][0].from_rank, c_helper->moves[i][0].from_file -'a')));
      }
      //black move
      if(!chessboard_update(boardState , &c_helper->moves[i][1], 'b')){
        ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("invalid move %s: \"%s\" piece : %c , rank : %i, file : %i", "chessboard cannot be update",  sanmove_to_str(&c_helper->moves[i][1]), c_helper->moves[i][1].piece, c_helper->moves[i][1].rank, c_helper->moves[i][1].file - 'a')));
      }
    }

    // if (result != 0) {
    //     ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    // }

    // // Convert the FEN string to a PostgreSQL text type
    // text *fen_text = cstring_to_text(fen);

    // Return the FEN representation of the board state
   PG_RETURN_CHESSBOARD_P(boardState);
}

PG_FUNCTION_INFO_V1(hasBoard);
Datum hasBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) ||  PG_ARGISNULL(1) ||  PG_ARGISNULL(2)) {
        ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED), errmsg("Null values are not allowed")));
    }

    Chessgame* chessgame = PG_GETARG_CHESSGAME_P(0);
    Chessboard* chessboard = PG_GETARG_CHESSBOARD_P(1);
    /*ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
        errmsg("invalid move %s: \"%s\"", "chessboard cannot be update", )));*/
    int32 half_moves = PG_GETARG_INT32(2); 
 
    bool ret_val = false; 


    if (half_moves < 0) {
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Invalid half-move count")));
    }
    Chessboard* chessboard_aftermove = getBoardPrivate(chessgame,half_moves);
    // I assume that getBoard will return null if the halfmoves invalid
    if (chessboard_aftermove)
    {
        char* chessboard_text = chessboard_to_str(chessboard);
        char *chessboard_aftermove_text = chessboard_to_str(chessboard_aftermove);

        // Comparing chessboard_text and chessboard_aftermove_text if there is no 
        //differences between them then ret_val becomes true, so the function does
        if (strcmp(chessboard_text,chessboard_aftermove_text) == 0)
        {
            ret_val = true;
        }
    }
    PG_RETURN_BOOL(ret_val);
}
