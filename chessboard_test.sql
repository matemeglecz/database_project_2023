DROP TABLE chessboardT;
DROP EXTENSION chess;

CREATE EXTENSION chess;
CREATE TABLE chessboardT (id integer, board chessboard);


INSERT INTO chessboardT values (1, 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'),(2, 'rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1');


select * from chessboardT;
