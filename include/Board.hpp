#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include "Common.hpp"

namespace bbc {

    struct Board {
        Board();
        // raw state
        U64                     bitboards[12];
        U64                     occupancies[3];
        int                     side;
        int                     enpassant;
        int                     castle;
        int                     ply;

        // formerly free functions → methods
        void parseFEN(const char *  fen);
        void print() const;

    };
    // take back and restore functions
    inline void save_board(Board& copy,  Board const& b)   {copy = b;}

    inline void restore_board( Board const& copy, Board& b)   {b = copy;}

}  // namespace bbc
