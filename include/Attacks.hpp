#pragma once
#include <cstdint>
#include <cstring> // memset

namespace bbc {

using U64 = uint64_t;

namespace Attacks {

    // -----------------------
    // File masks
    // -----------------------
    constexpr U64 not_a_file  = 18374403900871474942ULL;
    constexpr U64 not_h_file  = 9187201950435737471ULL;
    constexpr U64 not_hg_file = 4557430888798830399ULL;
    constexpr U64 not_ab_file = 18229723555195321596ULL;

    // -----------------------
    // Relevant bits
    // -----------------------
    constexpr int bishop_relevant_bits[64] = {
        6,5,5,5,5,5,5,6,
        5,5,5,5,5,5,5,5,
        5,5,7,7,7,7,5,5,
        5,5,7,9,9,7,5,5,
        5,5,7,9,9,7,5,5,
        5,5,7,7,7,7,5,5,
        5,5,5,5,5,5,5,5,
        6,5,5,5,5,5,5,6
    };

    constexpr int rook_relevant_bits[64] = {
        12,11,11,11,11,11,11,12,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        11,10,10,10,10,10,10,11,
        12,11,11,11,11,11,11,12
    };

    // -----------------------
    // Precomputed tables
    // -----------------------
    extern U64 pawn_attacks[2][64];
    extern U64 knight_attacks[64];
    extern U64 king_attacks[64];
    extern U64 bishop_masks[64];
    extern U64 rook_masks[64];
    extern U64 bishop_attacks[64][512];
    extern U64 rook_attacks[64][4096];
    extern U64 bishop_magic_numbers[64];
    extern U64 rook_magic_numbers[64];

    // -----------------------
    // API
    // -----------------------
    void init();

    inline U64 get_bishop(int square, U64 occupancy) {
        occupancy &= bishop_masks[square];
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64 - bishop_relevant_bits[square];
        return bishop_attacks[square][occupancy];
    }

    inline U64 get_rook(int square, U64 occupancy) {
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64 - rook_relevant_bits[square];
        return rook_attacks[square][occupancy];
    }

    inline U64 get_queen(int square, U64 occupancy) {
        return get_bishop(square, occupancy) | get_rook(square, occupancy);
    }

} // namespace Attacks
} // namespace bbc
