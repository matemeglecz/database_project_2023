-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION [votre_extension]" to load this file. \quit

/******************************************************************************
 * Input/Output
 ******************************************************************************/
CREATE EXTENSION chessboard

CREATE OR REPLACE FUNCTION chessboard_in(cstring)
  RETURNS ChessBoard
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_out(ChessBoard)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_recv(internal)
  RETURNS ChessBoard
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION chessboard_send(ChessBoard)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE ChessBoard (
  internallength = 64,
  input          = chessboard_in,
  output         = chessboard_out,
  receive        = chessboard_recv,
  send           = chessboard_send,
  alignment      = char
);

-- Conversion bidirectionnelle entre ChessBoard et texte
CREATE OR REPLACE FUNCTION chessboard_cast(text)
  RETURNS ChessBoard
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION text_cast(ChessBoard)
  RETURNS text
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CAST dans les deux directions
CREATE CAST (text AS ChessBoard) WITH FUNCTION chessboard_cast(text) AS IMPLICIT;
CREATE CAST (ChessBoard AS text) WITH FUNCTION text_cast(ChessBoard) AS IMPLICIT;

/******************************************************************************
 * Constructor (if needed)
 ******************************************************************************/
CREATE OR REPLACE FUNCTION chessboard(text)
  RETURNS ChessBoard
  AS 'MODULE_PATHNAME', 'chessboard_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- You can add constructor functions here if necessary.

/*****************************************************************************
 * Accessing values (if needed)
 *****************************************************************************/
CREATE OR REPLACE FUNCTION chessboard_get_piece(ChessBoard, integer)
  RETURNS char
  AS 'MODULE_PATHNAME', 'chessboard_get_piece'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

  CREATE OR REPLACE FUNCTION chessboard_get_row(ChessBoard, integer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'chessboard_get_row'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- You can add functions to access values if needed.

/******************************************************************************
 * Operators (if needed)
 ******************************************************************************/

-- You can add custom operators and functions for your type.

/******************************************************************************/

-- Additional functions and configurations can be added based on your requirements.
