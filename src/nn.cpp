/**
 * nnue_bbc.cpp
 * ------------
 * Non-template definitions for bbc::nnue.
 * Add this file to your build alongside nnue.cpp and misc.cpp.
 *
 * Build example:
 *   g++ -O3 -DUSE_AVX2 -mavx2 main.cpp nnue.cpp misc.cpp nnue_bbc.cpp -o engine
 */
 
#include "nn.hpp"
#include "nnue.hpp"

namespace bbc {
namespace nnue {
 
// Module-level flag — set to true once init() succeeds.
static bool g_loaded = false;
 
/**
 * Load the NNUE weights file from disk.
 * Call once at engine startup, before any search begins.
 *
 *   bbc::nnue::init("nn-xxxxxxxxxxxxxxxx.nnue");
 */
void init(const char* path) {
  nnue_init(path);   // delegates to the C probe library
  g_loaded = true;
}
 
/**
 * Returns true after a successful init() call.
 * Use this to guard evaluate() calls and fall back to HCE if needed.
 */
bool loaded() {
  return g_loaded;
}
 
/**
 * Evaluate a position supplied as a FEN string.
 * Returns centipawns from the side-to-move's perspective.
 *
 * Useful for quick testing from the UCI interface or unit tests:
 *   int score = bbc::nnue::evaluate_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
 */
int evaluate_fen(const char* fen) {
  return nnue_evaluate_fen(fen);
}
 
} // namespace nnue
} // namespace bbc
 
 
// ================================================================== //
//  INTEGRATION GUIDE                                                   //
// ================================================================== //
/*
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 1 — Startup                                               │
  └─────────────────────────────────────────────────────────────────┘
 
    In main.cpp or uci.cpp, before the search loop:
 
      #include "nnue_bbc.hpp"
      bbc::nnue::init("nn-xxxxxxxxxxxxxxxx.nnue");
 
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 2a — Mailbox board (piece[sq] array)                      │
  └─────────────────────────────────────────────────────────────────┘
 
    // Full replacement:
    int evaluate(const Board& b) {
      if (bbc::nnue::loaded())
        return bbc::nnue::evaluate(b);
      return hand_crafted_eval(b);  // fallback if no .nnue file
    }
 
    // Phase-blended hybrid (keep HCE influence in the opening):
    int evaluate(const Board& b) {
      int hce   = hand_crafted_eval(b);
      int nn    = bbc::nnue::evaluate(b);
      int phase = game_phase(b);    // e.g. 0=endgame, 256=opening
      return (hce * phase + nn * (256 - phase)) / 256;
    }
 
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 2b — Bitboard board                                       │
  └─────────────────────────────────────────────────────────────────┘
 
    int evaluate(int side) {
      return bbc::nnue::evaluate_bb(side, [&](auto cb) {
        for (int p = wP; p <= bK; p++) {
          uint64_t bb = bitboard[p];
          while (bb) {
            int sq = __builtin_ctzll(bb);
            bb &= bb - 1;
            cb(p, sq);
          }
        }
      });
    }
 
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 3 — Piece code table (CRITICAL — verify yours!)           │
  └─────────────────────────────────────────────────────────────────┘
 
    In nnue_bbc.hpp, edit detail::BBC_TO_NNUE[] so that:
 
      BBC_TO_NNUE[your_wK] == 1    BBC_TO_NNUE[your_bK] == 7
      BBC_TO_NNUE[your_wQ] == 2    BBC_TO_NNUE[your_bQ] == 8
      BBC_TO_NNUE[your_wR] == 3    BBC_TO_NNUE[your_bR] == 9
      BBC_TO_NNUE[your_wB] == 4    BBC_TO_NNUE[your_bB] == 10
      BBC_TO_NNUE[your_wN] == 5    BBC_TO_NNUE[your_bN] == 11
      BBC_TO_NNUE[your_wP] == 6    BBC_TO_NNUE[your_bP] == 12
 
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 4 — Square orientation                                    │
  └─────────────────────────────────────────────────────────────────┘
 
    nnue_evaluate() uses A1=0 … H8=63.
    If your engine uses A8=0 … H1=63, set detail::FLIP_SQUARE = true
    in nnue_bbc.hpp. This XORs each square with 56 (flips rank).
 
 
  ┌─────────────────────────────────────────────────────────────────┐
  │  STEP 5 — Build flags                                           │
  └─────────────────────────────────────────────────────────────────┘
 
    Compile nnue.cpp and misc.cpp alongside this file.
    For best performance, pick one SIMD target:
 
      -DUSE_AVX2 -mavx2      (modern x86 — recommended)
      -DUSE_SSE2 -msse2      (older x86)
      -DUSE_NEON             (Apple Silicon / ARM)
 
    Full example:
      g++ -O3 -DUSE_AVX2 -mavx2 \
          main.cpp nnue.cpp misc.cpp nnue_bbc.cpp -o engine
 
*/