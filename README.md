# KataFish - UCI Chess Engine (C++17)

## About
KataFish is a UCI chess engine written entirely in C++. This project is complete but is being updated as part of a long-term learning project. The engine is rated at roughly 2000 ELO based on games with known rated engines. If you want to learn more about chess programming, feel free to use this as a resource — there are many fundamental techniques used in this. If you simply want to run the engine, there is a **myengine.exe** file inside bin ready to use on chess-engine GUI platforms (Cute Chess, Arena, etc.) with UCI protocol.

---

## Statistics

Raw Nodes per second: ~**45 million NPS**   (Will vary based on hardware)

Search Nodes per second: ~**5 million NPS**

Search depth  (2000 ms): ~**11 - 16**   (Will vary based on game stage)

Approximate playing strength: ~**2000 ELO** (based on games vs Stockfish NNUE, tscp181, MadChessEngine)

---

## Basic Features

### Board Representation
- Bitboards
- Performance Test (perft)

### Search
- Alpha-Beta (Negamax) search
- Iterative-Deepening
- Quiescence search
- Transposition Table
- Null Move pruning
- Move ordering (Killer moves, MVV/LVA)

### Evaluation
- Material
- Piece-square tables 

### Other
- UCI protocol support
- Time management

---

## Modules / Roadmap
- **Common.hpp** — Contains common functions used across modules (bit helpers, time helpers, enums)
- **Board.hpp** — Core board representation (bitboards, state info)
- **Attacks.hpp** — Precomputed attacks from each piece from each square (bitmasks, magic hashing)
- **Move.hpp** — Core move representation (target/source square, enpassant, castling, promotion, captures)
- **MoveGen.hpp** — Generation of legal moves (generate moves, make moves, attacked squares)
- **Perft.hpp** — Testing of raw move generation (perft driver, nodes per second)
- **Eval.hpp** — Static evaluation (material balance, piece-square tables).
- **Search.hpp** — Core search functions (negamax/alpha-beta, iterative deepening, quiescence, etc.)
- **TT.hpp** — Transposition table memory for encountered moves (Zobrist hashing, probing).
- **MoveOrder.hpp** — Move ordering inside negamax search (MVV/LVA, killer moves, move scoring, insertion and quick sort)
- **Engine.hpp** — Important context for engine to run (time left, nodes explored, etc.)
- **UCI.hpp** — Handles communication with GUI (init positions, read time remaining, output best move).  

---

## Acknowledgements

* Chess Programming Wiki (Maksim Korzh), Stockfish (Open-source engine) for code ideas
* CuteChess / Arena authors for testing GUIs


