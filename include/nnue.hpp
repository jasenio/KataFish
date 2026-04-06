#ifndef NNUE_H
#define NNUE_H

#include <stdbool.h>
#include <stdalign.h>

#include "misc.hpp"
#include "Board.hpp"

#ifdef __cplusplus
#   define EXTERNC extern "C"
#else
#   define EXTERNC
#endif
#if defined (_WIN32)
#   define _CDECL __cdecl
#ifdef DLL_EXPORT
#   define DLLExport EXTERNC __declspec(dllexport)
#else
#   define DLLExport EXTERNC __declspec(dllimport)
#endif
#else
#   define _CDECL
#   define DLLExport EXTERNC
#endif

namespace bbc{
// ------------------------------------------------------------------ //
//  Internal piece-code conversion                                     //
//  Adapt the constants below to match YOUR engine's enum values.      //
// ------------------------------------------------------------------ //
namespace detail {
 
  // --- YOUR ENGINE PIECE CONSTANTS (edit to match your enums) ---
  //
  // Example layout assumed here:
  //   enum Piece { EMPTY=0, wP, wN, wB, wR, wQ, wK,
  //                          bP, bN, bB, bR, bQ, bK };
  //   enum Color  { WHITE=0, BLACK=1 };
  //
  // nnue_evaluate() wants:
  //   wK=1, wQ=2, wR=3, wB=4, wN=5, wP=6
  //   bK=7, bQ=8, bR=9, bB=10, bN=11, bP=12
 
  // Map from your internal piece index → nnue piece code.
  // Index 0 = EMPTY → 0 (ignored).
  // Adjust the array body to match your Piece enum order.
//   constexpr int BBC_TO_NNUE[13] = {
//   //  EMPTY  wP  wN  wB  wR  wQ  wK   bP  bN  bB  bR  bQ  bK
//         0,    6,  5,  4,  3,  2,  1,   12, 11, 10,  9,  8,  7
//   };
    constexpr int BBC_TO_NNUE[12] = {
        6,  // P
        5,  // N
        4,  // B
        3,  // R
        2,  // Q
        1,  // K
        12, // p
        11, // n
        10, // b
        9,  // r
        8,  // q
        7   // k
    };

 
  // Flip square vertically so A8=0…H1=63 → A1=0…H8=63 if needed.
  // If your squares are already A1=0, set FLIP_SQUARE = false.
  constexpr bool FLIP_SQUARE = true;
 
  inline int to_nnue_sq(int sq) {
    return FLIP_SQUARE ? (sq ^ 56) : sq;
  }
 
} // namespace detail
 

/*pieces*/
const int pic_tab[14] = {
    blank,king,queen,rk,bish,knight,pawn,
    king,queen,rk,bish,knight,pawn,blank
};

#define PIECE(x)         (pic_tab[x])
#define COMBINE(c,x)     ((x) + (c) * 6) 


int nnue_evaluate_pos(Board& board);

/**
* Load NNUE file
*/

#ifdef __cplusplus
extern "C" {
#endif

void nnue_init(
  const char * evalFile             /** Path to NNUE file */
);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
* Evaluate on FEN string
*/
int nnue_evaluate_fen(
  const char* fen                   /** FEN string to probe evaluation for */
);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
extern "C" {
#endif
/**
* Evaluation subroutine suitable for chess engines.
* -------------------------------------------------
* Piece codes are
*     wking=1, wqueen=2, wrook=3, wbishop= 4, wknight= 5, wpawn= 6,
*     bking=7, bqueen=8, brook=9, bbishop=10, bknight=11, bpawn=12,
* Square are
*     A1=0, B1=1 ... H8=63
* Input format:
*     piece[0] is white king, square[0] is its location
*     piece[1] is black king, square[1] is its location
*     ..
*     piece[x], square[x] can be in any order
*     ..
*     piece[n+1] is set to 0 to represent end of array
*/
int nnue_evaluate(
  int player,                       /** Side to move */
  int* pieces,                      /** Array of pieces */
  int* squares                      /** Corresponding array of squares the piece stand on */
);
}
#ifdef __cplusplus
}
#endif

#endif