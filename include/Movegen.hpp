#pragma once
#include <string>
#include <cstdint>

// Pull in your existing engine-wide declarations.
// Adjust these include paths to your project layout.
#include "Common.hpp"   // piece enums, squares, U64, bitboards/occupancies externs (or Board.hpp exposes them)
#include "Board.hpp"    // Board struct, save_board / restore_board
#include "Attacks.hpp"  // get_bishop_attacks / get_rook_attacks / get_queen_attacks, attack tables
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
// Return 1 if `square` is attacked by `side`, else 0
int is_square_attacked(const Board& board, int square, int side_);
// -----------------------------
// Printing / debugging
// -----------------------------
void print_attacked_squares(const Board& b, int side);

// Move types selector
enum { all_moves, only_captures };

// Checkmate helpers
bool in_check_now(const Board& b);

bool has_legal_move(const Board & b);

// Generate pseudo-legal moves for current global `side`
void generate_moves(MoveList& list, Board& b, bool quiet=true);

// Make a move; if move_flag==all_moves, try any; if only_captures, reject non-captures.
// Returns 1 if legal (applied), 0 if illegal (board restored).
int make_move(int move, int move_flag, Board& b, StateInfo& st);


// Null moves
void make_null_move(Board& board, StateInfo& st);

void restore_null(Board& board, StateInfo& st);

} // namespace bbc


