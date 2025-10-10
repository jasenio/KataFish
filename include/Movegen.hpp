#pragma once
#include <string>
#include <cstdint>

#include "Common.hpp"   
#include "Board.hpp"   
#include "Attacks.hpp"  
#include "Move.hpp"

namespace bbc {

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

// -----------------------------
// Generation & make-move API
// -----------------------------

// return 1 if square is attacked by side_, otherwise 0
int is_square_attacked(const Board& board, int square, int side_);

// -----------------------------
// Printing / debugging
// -----------------------------
void print_attacked_squares(const Board& b, int side);

// move types selector
enum { all_moves, only_captures };

// checkmate helpers
bool in_check_now(const Board& b);

bool has_legal_move(const Board & b);

// generate pseudo-legal moves for current global `side`
void generate_moves(MoveList& list, Board& b, bool quiet=true);

// Make a move; if move_flag==all_moves, try any; if only_captures, reject non-captures.
// returns 1 if legal (applied), 0 if illegal (board restored).
int make_move(int move, int move_flag, Board& b, StateInfo& st);


// null moves
void make_null_move(Board& board, StateInfo& st);

void restore_null(Board& board, StateInfo& st);

} // end namespace bbc


