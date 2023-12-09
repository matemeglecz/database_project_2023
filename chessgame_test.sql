DROP TABLE t;
DROP EXTENSION chess;

CREATE EXTENSION chess;
CREATE TABLE t (id integer, g chessgame);


INSERT INTO t values (5, '1. e4 e5'),(6, '1. e4 d5 2. exd5 Qxd 3. Nc3 Qd8 4. Bc4 Nf6 5. Nf3 Bg4 6. h3 Bxf3 7. Qxf3 e6 8. Qxb7 Nbd7 9. Nb5 Rc8 10. Nxa7 Nb6 11. Nxc8 Nxc8 12. d4 Nd6 13. Bb5+ Nxb5 14. Qxb5+ Nd7 15. d5 exd5 16. Be3 Bd6 17. Rd1 Qf6 18. Rxd5 Qg6 19. Bf4 Bxf4 20. Qxd7+ Kf8 21. Qd8#');


select * from t;

select g as g, getFirstMoves(g, 3) as first_moves, hasOpening(g, '1. d5') as openingwrong, hasOpening(g, '1. e4') as openingright, hasOpening(g, '1. e4 d5 2. exd5 Qxd') as long_true, hasOpening(g, '1. e4 d6 2. exd5 Qxd') as long_false from t;

create index openeing_idx on t(g);

EXPLAIN ANALYZE SELECT count(*)
FROM t
WHERE hasopening(t, '1. e4'); 