#!/bin/sh
# automaticall plays a match between two engines using xboard and save the
# result to a file, good for testing and tuning strength

OUTFILE=xboard_games.txt
GAMES=10
PROGRAM1="./smolchess -x"
PROGRAM2="./smolchess -x"
TIME="0:30"

rm $OUTFILE

xboard -matchPause 100 -ringBellAfterMoves false -ponderNextMove false -tc $TIME -fcp "$PROGRAM1" -scp "$PROGRAM2" -matchGames $GAMES -saveGameFile $OUTFILE  2>> $OUTFILE 
