#pragma once
#include "Common.hpp"
#include "Attacks.hpp"

namespace bbc{
// -----------------------------
// Move encoding
// -----------------------------
inline constexpr int encode_move(
    int source, int target, int piece, int promoted,
    bool capture, bool double_push, bool enpassant, bool castling) noexcept
{
    return  (int(source)   & 0x3f) |
           ((int(target)   & 0x3f) << 6)  |
           ((int(piece)    & 0x0f) << 12) |
           ((int(promoted) & 0x0f) << 16) |
           ((capture      ? 1u : 0u) << 20) |
           ((double_push  ? 1u : 0u) << 21) |
           ((enpassant    ? 1u : 0u) << 22) |
           ((castling     ? 1u : 0u) << 23);
}

inline constexpr int  get_move_source(int m)     noexcept { return  (m & 0x3f); }
inline constexpr int  get_move_target(int m)     noexcept { return ((m >> 6) & 0x3f); }
inline constexpr int  get_move_piece(int m)      noexcept { return ((m >> 12) & 0xf); }
inline constexpr int  get_move_promoted(int m)   noexcept { return ((m >> 16) & 0xf); }
inline constexpr bool get_move_capture(int m)    noexcept { return  m & 0x100000; }
inline constexpr bool get_move_double(int m)     noexcept { return  m & 0x200000; }
inline constexpr bool get_move_enpassant(int m)  noexcept { return  m & 0x400000; }
inline constexpr bool get_move_castling(int m)   noexcept { return  m & 0x800000; }

// -----------------------------
// Move container
// -----------------------------
struct MoveList{
    int moves[256];
    int scores[256];
    int count;
};

// Add one encoded move into the list
void add_move(MoveList& list, int move);

// -----------------------------
// Printing / debugging
// -----------------------------
void print_move(int move);
std::string move_string(int move);
void print_move_list(MoveList& list);

// -----------------------------
// Debug list compare helpers
// -----------------------------
bool ensure_same(int a[], int b[], int length = 256);
void print_two_lists(int a[], int b[], int length = 256);


}