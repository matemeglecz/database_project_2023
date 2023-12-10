DROP TABLE t;
DROP EXTENSION chess;

CREATE EXTENSION chess;
CREATE TABLE t (id integer, g chessgame);


INSERT INTO t values (5, '1. e4 e5'),(6, '1. e4 d5 2. exd5 Qxd 3. Nc3 Qd8 4. Bc4 Nf6 5. Nf3 Bg4 6. h3 Bxf3 7. Qxf3 e6 8. Qxb7 Nbd7 9. Nb5 Rc8 10. Nxa7 Nb6 11. Nxc8 Nxc8 12. d4 Nd6 13. Bb5+ Nxb5 14. Qxb5+ Nd7 15. d5 exd5 16. Be3 Bd6 17. Rd1 Qf6 18. Rxd5 Qg6 19. Bf4 Bxf4 20. Qxd7+ Kf8 21. Qd8#');


select g as g, getFirstMoves(g, 3) as first_moves, hasOpening(g, '1. d5') as openingwrong, hasOpening(g, '1. e4') as openingright from t;
select * from t;

-- Step 1: Create a function to generate random chess games
CREATE OR REPLACE FUNCTION random_game() RETURNS text AS $$
DECLARE
  moves text[] := ARRAY['1. e4', '1. d4', '1. Nf3', '1. c4'];
BEGIN
  RETURN moves[1+random()*(array_length(moves, 1)-1)::int] || ' e5 2. Nf3 Nc6 3. Bb5';
END;
$$ LANGUAGE plpgsql;

-- Step 2: Insert a large number of random games into your table
DO $$
BEGIN
  FOR i IN 1..200 LOOP
    INSERT INTO t (g) VALUES (random_game());
  END LOOP;
END;
$$;

SELECT COUNT(*) FROM t;

EXPLAIN ANALYZE SELECT COUNT(*) FROM t WHERE hasOpening(g, '1. e4');

create index t_idx on t(g);

EXPLAIN ANALYZE SELECT COUNT(*) FROM t WHERE hasOpening(g, '1. e4');