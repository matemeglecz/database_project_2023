-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION chess" to load this file. \quit

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE OR REPLACE FUNCTION chessgame_in(cstring)
  RETURNS chessgame
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_out(chessgame)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_recv(internal)
  RETURNS chessgame
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_send(chessgame)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE chessgame (
  internallength = 2000,
  input          = chessgame_in,
  output         = chessgame_out,
  receive        = chessgame_recv,
  send           = chessgame_send,
  alignment      = char
);

CREATE OR REPLACE FUNCTION chessgame(text)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'chessgame_cast_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text(chessgame)
  RETURNS text
  AS 'MODULE_PATHNAME', 'chessgame_cast_to_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (text as chessgame) WITH FUNCTION chessgame(text) AS IMPLICIT;
CREATE CAST (chessgame as text) WITH FUNCTION text(chessgame);

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION chessgame(cstring)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'chessgame_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Accessing values
 *****************************************************************************/

 CREATE FUNCTION getFirstMoves(chessgame, integer)
  RETURNS chessgame
  AS 'MODULE_PATHNAME', 'getFirstMoves'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION hasOpening(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'hasOpening'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

  /******************************************************************************/

/* B-Tree comparison functions */

CREATE OR REPLACE FUNCTION chessgame_eq(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_lt(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_le(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_gt(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessgame_ge(chessgame, chessgame)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/******************************************************************************/

/* B-Tree comparison operators */

CREATE OPERATOR = (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_eq,
  COMMUTATOR = =, NEGATOR = <>
);
CREATE OPERATOR < (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = chessgame, RIGHTARG = chessgame,
  PROCEDURE = chessgame_gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OR REPLACE FUNCTION hasOpening(val1 chessgame, val2 chessgame) RETURNS boolean AS $$
  BEGIN
  RETURN (val2 <= val1);
  END; $$
  LANGUAGE PLPGSQL;

/******************************************************************************/

/* B-Tree support function */

CREATE OR REPLACE FUNCTION chessgame_cmp(chessgame, chessgame)
  RETURNS integer
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

/* B-Tree operator class */

CREATE OPERATOR CLASS chessgame_ops
DEFAULT FOR TYPE chessgame USING btree
AS
        OPERATOR        1       <  ,
        OPERATOR        2       <= ,
        OPERATOR        3       =  ,
        OPERATOR        4       >= ,
        OPERATOR        5       >  ,
        FUNCTION        1       chessgame_cmp(chessgame, chessgame);

/******************************************************************************/