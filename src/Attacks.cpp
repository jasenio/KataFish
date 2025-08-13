#include "Attacks.hpp"
#include "Common.hpp"

namespace bbc {
namespace Attacks {

// ------------------
// Table definitions
// ------------------
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 bishop_magic_numbers[64];
U64 rook_magic_numbers[64];

// ------------------
// Bit helpers
// ------------------
inline void set_bit(U64 &bb, int sq) { bb |= (1ULL << sq); }
inline void pop_bit(U64 &bb, int sq) { bb &= ~(1ULL << sq); }

// ------------------
// Attack masking
// ------------------
static U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL, bitboard = 0ULL;
    set_bit(bitboard, square);

    if (side == 0) { // white
        if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    } else { // black
        if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    }
    return attacks;
}

static U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL, bitboard = 0ULL;
    set_bit(bitboard, square);

    if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
    if ((bitboard >> 6)  & not_ab_file) attacks |= (bitboard >> 6);
    if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
    if ((bitboard << 6)  & not_hg_file) attacks |= (bitboard << 6);

    return attacks;
}

static U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL, bitboard = 0ULL;
    set_bit(bitboard, square);

    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);

    return attacks;
}

static U64 mask_bishop_attacks(int square) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;

    for (int r = tr+1, f = tf+1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r*8 + f));
    for (int r = tr-1, f = tf+1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r*8 + f));
    for (int r = tr+1, f = tf-1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r*8 + f));
    for (int r = tr-1, f = tf-1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r*8 + f));

    return attacks;
}

static U64 mask_rook_attacks(int square) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;

    for (int r = tr+1; r <= 6; r++) attacks |= (1ULL << (r*8 + tf));
    for (int r = tr-1; r >= 1; r--) attacks |= (1ULL << (r*8 + tf));
    for (int f = tf+1; f <= 6; f++) attacks |= (1ULL << (tr*8 + f));
    for (int f = tf-1; f >= 1; f--) attacks |= (1ULL << (tr*8 + f));

    return attacks;
}

// ------------------
// Sliding attacks on the fly
// ------------------
static U64 bishop_attacks_otf(int square, U64 block) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;

    for (int r = tr+1, f = tf+1; r <= 7 && f <= 7; r++, f++) { attacks |= (1ULL << (r*8 + f)); if ((1ULL << (r*8 + f)) & block) break; }
    for (int r = tr-1, f = tf+1; r >= 0 && f <= 7; r--, f++) { attacks |= (1ULL << (r*8 + f)); if ((1ULL << (r*8 + f)) & block) break; }
    for (int r = tr+1, f = tf-1; r <= 7 && f >= 0; r++, f--) { attacks |= (1ULL << (r*8 + f)); if ((1ULL << (r*8 + f)) & block) break; }
    for (int r = tr-1, f = tf-1; r >= 0 && f >= 0; r--, f--) { attacks |= (1ULL << (r*8 + f)); if ((1ULL << (r*8 + f)) & block) break; }

    return attacks;
}

static U64 rook_attacks_otf(int square, U64 block) {
    U64 attacks = 0ULL;
    int tr = square / 8, tf = square % 8;

    for (int r = tr+1; r <= 7; r++) { attacks |= (1ULL << (r*8 + tf)); if ((1ULL << (r*8 + tf)) & block) break; }
    for (int r = tr-1; r >= 0; r--) { attacks |= (1ULL << (r*8 + tf)); if ((1ULL << (r*8 + tf)) & block) break; }
    for (int f = tf+1; f <= 7; f++) { attacks |= (1ULL << (tr*8 + f)); if ((1ULL << (tr*8 + f)) & block) break; }
    for (int f = tf-1; f >= 0; f--) { attacks |= (1ULL << (tr*8 + f)); if ((1ULL << (tr*8 + f)) & block) break; }

    return attacks;
}

// ------------------
// Occupancy
// ------------------
static U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;
    for (int count = 0; count < bits_in_mask; count++) {
        int square = __builtin_ctzll(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1 << count)) occupancy |= (1ULL << square);
    }
    return occupancy;
}

// ------------------
// Init helpers
// ------------------
static void init_leapers() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[0][square] = mask_pawn_attacks(0, square);
        pawn_attacks[1][square] = mask_pawn_attacks(1, square);
        knight_attacks[square]  = mask_knight_attacks(square);
        king_attacks[square]    = mask_king_attacks(square);
    }
}

static void init_sliders(int bishop) {
    for (int square = 0; square < 64; square++) {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square]   = mask_rook_attacks(square);

        U64 mask = bishop ? bishop_masks[square] : rook_masks[square];
        int bits = __builtin_popcountll(mask);
        int variations = 1 << bits;

        for (int index = 0; index < variations; index++) {
            U64 occ = set_occupancy(index, bits, mask);
            int magic_index;
            if (bishop) {
                magic_index = (occ * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] = bishop_attacks_otf(square, occ);
            } else {
                magic_index = (occ * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                rook_attacks[square][magic_index] = rook_attacks_otf(square, occ);
            }
        }
    }
}

void init() {
    // fill rook_magic_numbers[] and bishop_magic_numbers[] with your baked values
    init_leapers();
    init_sliders(1);
    init_sliders(0);
}

} // namespace Attacks
} // namespace bbc
