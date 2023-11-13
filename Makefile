MODULE_big	= complex
OBJS = \
	$(WIN32RES) \
	complex.o \
	complex_btree.o

EXTENSION   = complex
DATA        = complex--1.0.sql
HEADERS_complex = complex.h

PG_CONFIG ?= pg_config
PGXS = $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
