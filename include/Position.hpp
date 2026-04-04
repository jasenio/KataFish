#pragma once
#include "Common.hpp"   
#include "Board.hpp"   
#include "Attacks.hpp"  
#include "Move.hpp"


// responsible for making and taking back moves, with legality checks and efficient updates

namespace bbc{

inline constexpr int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

// return 1 if square is attacked by side_, otherwise 0
int is_square_attacked(const Board& board, int square, int side_);

// Make a move; if move_flag==all_moves, try any; if only_captures, reject non-captures.
// returns 1 if legal (applied), 0 if illegal (board restored).

// make_move with efficient updates
int make_move(int move, int move_flag, Board& b, StateInfo& st);

// undo make_move while efficiently reversing updates
void undo_move(Board& b, const StateInfo& st, const int move);

// null moves
void make_null_move(Board& board, StateInfo& st);

void restore_null(Board& board, StateInfo& st);

}