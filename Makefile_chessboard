MODULE_big	= chessboard
OBJS = \
	$(WIN32RES) \
	chessboard.o 

EXTENSION   = chessboard
DATA        = chessboard--1.0.sql
HEADERS_chessboard = chessboard.h

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)