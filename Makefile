MODULE_big = chessBoard
OBJS = chessBoard.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)