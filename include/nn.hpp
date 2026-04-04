#pragma once
 
/**
 * nnue_bbc.hpp
 * ------------
 * Drop-in NNUE integration for the bbc chess engine.
 *
 * Usage:
 *   1. #include "nnue_bbc.hpp"  (after your board/piece definitions)
 *   2. Call bbc::nnue::init("nn-xxxxxxxxxxxxxxxx.nnue") at startup.
 *   3. Replace or augment your eval() with bbc::nnue::evaluate(board).
 *
 * Piece code mapping expected by nnue_evaluate():
 *   wK=1  wQ=2  wR=3  wB=4  wN=5  wP=6
 *   bK=7  bQ=8  bR=9  bB=10 bN=11 bP=12
 *   terminator = 0
 *
 * Square mapping: A1=0, B1=1 ... H8=63
 */
 
#include <cstring>
#include <cstdio>
#include "nnue.hpp"

// The original nnue probe API (C linkage)
extern "C" {
  void nnue_init(const char* evalFile);
  int  nnue_evaluate(int player, int* pieces, int* squares);
  int  nnue_evaluate_fen(const char* fen);
}
 
namespace bbc {
 

// ------------------------------------------------------------------ //
//  NNUE namespace                                                      //
// ------------------------------------------------------------------ //
namespace nnue {
 
  // Defined in nnue_bbc.cpp
  void init(const char* path);
  bool loaded();
  int  evaluate_fen(const char* fen);
 
  // ---------------------------------------------------------------- //
  //  Template: mailbox board (piece[sq] array)                        //
  //                                                                   //
  //  Board must expose:                                               //
  //    board.side       – 0=WHITE, 1=BLACK                            //
  //    board.piece[sq]  – your piece code on square sq (0 if empty)   //
  // ---------------------------------------------------------------- //
  template<typename Board>
  inline int evaluate(Board& board) {
    return nnue_evaluate_pos(board);
    // int pieces[34]  = {};
    // int squares[34] = {};
    // int idx = 2; // 0=wK, 1=bK, 2..=everything else
 
    // for (int sq = 0; sq < 64; sq++) {
    //   int pc = board.piece_at[sq];
    //   if (pc == -1  ) continue; // -1 == no_piece
 
    //   int nnue_pc = detail::BBC_TO_NNUE[pc];
    //   int nnue_sq = detail::to_nnue_sq(sq);
 
    //   if (nnue_pc == 1) {          // white king
    //     pieces[0]  = 1;
    //     squares[0] = nnue_sq;
    //   } else if (nnue_pc == 7) {   // black king
    //     pieces[1]  = 7;
    //     squares[1] = nnue_sq;
    //   } else {
    //     pieces[idx]  = nnue_pc;
    //     squares[idx] = nnue_sq;
    //     idx++;
    //   }
    // }
 
    // pieces[idx] = squares[idx] = 0; // terminator
    // return nnue_evaluate(board.side, pieces, squares);
  }


 
  // ---------------------------------------------------------------- //
  //  Template: bitboard board                                         //
  //                                                                   //
  //  Provide a lambda that iterates all occupied squares and calls    //
  //  cb(int your_piece_code, int sq) for each one.                   //
  //                                                                   //
  //  Example:                                                         //
  //    bbc::nnue::evaluate_bb(side, [&](auto cb) {                   //
  //      for (int p = wP; p <= bK; p++) {                            //
  //        uint64_t bb = bitboard[p];                                 //
  //        while (bb) {                                               //
  //          int sq = __builtin_ctzll(bb); bb &= bb - 1;             //
  //          cb(p, sq);                                               //
  //        }                                                          //
  //      }                                                            //
  //    });                                                            //
  // ---------------------------------------------------------------- //
  // template<typename Fn>
  // inline int evaluate_bb(int side, Fn each_piece) {
  //   int pieces[34]  = {};
  //   int squares[34] = {};
  //   int idx = 2;
 
  //   each_piece([&](int pc, int sq) {
  //     int nnue_pc = detail::BBC_TO_NNUE[pc];
  //     int nnue_sq = detail::to_nnue_sq(sq);
 
  //     if (nnue_pc == 1) {
  //       pieces[0]  = 1;  squares[0] = nnue_sq;
  //     } else if (nnue_pc == 7) {
  //       pieces[1]  = 7;  squares[1] = nnue_sq;
  //     } else if (idx < 32) {
  //       pieces[idx]  = nnue_pc;
  //       squares[idx] = nnue_sq;
  //       idx++;
  //     }
  //   });
 
  //   pieces[idx] = squares[idx] = 0;
  //   return nnue_evaluate(side, pieces, squares);
  // }
 
} // namespace nnue
} // namespace bbc