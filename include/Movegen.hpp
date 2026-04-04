#pragma once
#include <string>
#include <cstdint>

#include "Common.hpp"   
#include "Board.hpp"   
#include "Attacks.hpp"  
#include "Move.hpp"
#include "Position.hpp"

namespace bbc {

// -----------------------------
// Generation & make-move API
// -----------------------------

// -----------------------------
// Printing / debugging
// -----------------------------
void print_attacked_squares(const Board& b, int side);


// checkmate helpers
bool in_check_now(const Board& b);

bool has_legal_move(const Board & b);

// generate pseudo-legal moves for current global `side`
void generate_moves(MoveList& list, Board& b, bool quiet=true);


} // end namespace bbc


