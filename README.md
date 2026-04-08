# KataFish 2.4 - UCI Chess Engine (C++17)

## About
KataFish is a UCI chess engine written entirely in C++. This project is complete but is being updated as part of a long-term learning project (started in August 2024). If you want to learn more about chess programming, feel free to use this as a resource — there are many fundamental techniques used in this. If you simply want to test the engine, there is a **katafish.exe** file inside /bin ready to use on chess-engine GUI platforms (Cute Chess, Arena, etc.) with UCI protocol.

**kata** *(型・形, noun, Japanese)*  
-A form, pattern, or prescribed sequence of movements.  
-A method of structured practice used to develop skill through repetition and refinement.  
-(By extension) A disciplined approach to mastering complex systems through iteration.

## Quick Start (Make)

Requirements:
- `g++` and `make` for Linux/macOS builds
- `x86_64-w64-mingw32-g++-posix` for Windows cross-builds (`std::thread` support)

Common commands:
```bash
make help
make win-static
make run
```

Outputs:
- `bin/katafish.exe` (Windows static)

---

## Statistics

Raw Nodes per second: ~**140 million NPS**   (Bulk-counting, will vary based on hardware obviously)

Search Nodes per second: ~**300K NPS** (Also varies on hardware)

Search depth  (2000 ms): ~**7-8**   (Will vary based on position and hardware!)

Approximate playing strength: ~**2550 CCRL ELO ([Top 400](https://computerchess.org.uk/4040/)!)** (based on games with [Stash-20](https://dannyhammer.github.io/engine-testing-guide/determining-strength.html))

```
Katafish 2.4 vs Stash-20
Score of Katafish 2.4 vs Stash-20: 341 - 245 - 111  [0.569] 697
...      Katafish 2.4 playing White: 204 - 84 - 61  [0.672] 349
...      Katafish 2.4 playing Black: 137 - 161 - 50  [0.466] 348
...      White vs Black: 365 - 221 - 111  [0.603] 697
Elo difference: 48.2 +/- 23.8, LOS: 100.0 %, DrawRatio: 15.9 %
SPRT: llr 2.96 (100.4%), lbound -2.94, ubound 2.94 - H1 was accepted
```

See SPRT.md for more results
---

## Basic Features

### Board Representation
- Bitboards
- Position
- do_move/undo_move
- Performance Test (perft)

### Search
- Alpha-Beta (Negamax) search
- Iterative-Deepening
- Transposition Table
- Principal Variation Search (PVS)
- Quiescence search
- Null Move Pruning
- Move ordering (Killer moves, MVV/LVA)

### Evaluation
- Material
- Efficiently Updatable Neural Networks (NNUE-Lazy)
- Threefold Repetition

### Other
- UCI protocol support
- Time management
- Lichess-Bot ([See me play](https://lichess.org/@/KataFish))
---

## Modules / Roadmap
- **Common.hpp** — Contains common functions used across modules (bit helpers, time helpers, enums)
- **Board.hpp** — Core board representation (bitboards, state info)
- **Position.hpp** — Core board updates (do_move/undo_move, check legal moves)
- **Attacks.hpp** — Precomputed attacks from each piece from each square (bitmasks, magic hashing)
- **Move.hpp** — Core move representation (target/source square, enpassant, castling, promotion, captures)
- **MoveGen.hpp** — Generation of legal moves (generate moves, make moves, attacked squares)
- **Perft.hpp** — Testing of raw move generation (perft driver, nodes per second)
- **Eval.hpp** — Static evaluation (material balance, piece-square tables, Threefold Repetition).
- **Search.hpp** — Core search functions (negamax/alpha-beta, iterative deepening, quiescence, etc.)
- **TT.hpp** — Transposition table memory for encountered moves (Zobrist hashing, probing).
- **MoveOrder.hpp** — Move ordering inside negamax search (MVV/LVA, killer moves, move scoring, insertion and quick sort)
- **Engine.hpp** — Important context for engine to run (time left, nodes explored, etc.)
- **UCI.hpp** — Handles communication with GUI (init positions, read time remaining, output best move).  
- **nnue.hpp** — Handles efficient updates using halfKP architecture, forward propagation, and dirty piece updates

---

## Future Work
### TO DO
- Update readme (additions) + Add statistics (ELO)
- Add tutorial on NNUE + findings (What did I do? features==>layers, dirtyPieces, etc.)
- Add blog about Chess Resources (traditional vs using AI today)
- Ablation Studies
- Train personal NNUE
- Multi-Threaded Search
---

## Acknowledgements

* Chess Programming Wiki (Maksim Korzh), Stockfish (Open-source engine)
* CuteChess / Arena authors for testing GUIs and SPRT testing
* Artificial Intelligence: A Modern Approach (Peter Norvig)
