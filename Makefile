EXTENSION = chessboard
MODULES = chessboard
DATA = chessboard--1.0.sql chessboard.control
OBJS = chessboard.o 

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)