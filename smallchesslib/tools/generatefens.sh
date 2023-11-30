#!/bin/sh
# generates random FENs with stockfish evaluations after ';'

for i in $(seq 1 1 1000)
do
  fen=$(./randomfen)
  score=$(./stockfisheval.sh "$fen")
  echo "$fen;$score"
  sleep 1 # relieve CPU, you can comment this out
done
