#!/bin/sh
# evaluates given FEN with stockfish
# stockfish is pretty retarded, it can't give absolute evaluation so we have to convert the values lol

f="$1"

MATE_VAL=1000000
EVAL_TIME_LIMIT=1

s=`(echo "position fen $f"; echo "go infinite"; sleep $EVAL_TIME_LIMIT) | stockfish | grep -oEh "(score cp (-?[0-9]+)|mate [^ ]+ )" | sed "s/mate -.*/-$MATE_VAL/g" | sed "s/mate .*/$MATE_VAL/g" | sed "s/score cp //g" | tail -1`

if echo "$f" | grep -q " b "; then
  r=$(( -1 * s ))
  echo "$r"
else
  echo "$s"
fi
