#include "Move.hpp"
#include <string>
#include <iostream>


namespace bbc{

// -----------------------------
// Add move
// -----------------------------
void add_move(MoveList& list, int move) {
    list.moves[list.count++] = move;
}

// -----------------------------
// Printing helpers
// -----------------------------
void print_move(int move) {
    std::printf("%s%s%c\n", square_to_coordinates[get_move_source(move)],
                        square_to_coordinates[get_move_target(move)],
                        promoted_pieces[get_move_promoted(move)]);
}

std::string move_string(int move) {
    return std::string(square_to_coordinates[get_move_source(move)]) +
           std::string(square_to_coordinates[get_move_target(move)]) +
           std::string(1, promoted_pieces[get_move_promoted(move)]);
}

void print_move_list(MoveList& list) {
    if (!list.count) { std::printf("\n     No move in the move list!\n"); return; }

    std::printf("\n     move    piece     capture   double    enpass    castling\n\n");
    for (int i = 0; i < list.count; ++i) {
        int m = list.moves[i];
#ifdef WIN64
        std::printf("      %s%s%c   %c         %d         %d         %d         %d\n",
            square_to_coordinates[get_move_source(m)],
            square_to_coordinates[get_move_target(m)],
            get_move_promoted(m) ? promoted_pieces[get_move_promoted(m)] : ' ',
            ascii_pieces[get_move_piece(m)],
            get_move_capture(m) ? 1 : 0,
            get_move_double(m) ? 1 : 0,
            get_move_enpassant(m) ? 1 : 0,
            get_move_castling(m) ? 1 : 0);
#else
        std::printf("     %s%s%c   %s         %d         %d         %d         %d\n",
            square_to_coordinates[get_move_source(m)],
            square_to_coordinates[get_move_target(m)],
            get_move_promoted(m) ? promoted_pieces[get_move_promoted(m)] : ' ',
            unicode_pieces[get_move_piece(m)],
            get_move_capture(m) ? 1 : 0,
            get_move_double(m) ? 1 : 0,
            get_move_enpassant(m) ? 1 : 0,
            get_move_castling(m) ? 1 : 0);
#endif
    }
    std::printf("\n\n     Total number of moves: %d\n\n", list.count);
}

// -----------------------------
// Debug list compare helpers
// -----------------------------
static void sort_desc(int* a, int len) {
    for (int i = 0; i < len; ++i) {
        int max_idx = i;
        for (int j = i; j < len; ++j) if (a[j] > a[max_idx]) max_idx = j;
        std::swap(a[i], a[max_idx]);
    }
}

bool ensure_same(int a[], int b[], int length) {
    // copy to avoid side-effects? The original mutated; we keep behavior.
    sort_desc(a, length);           // original first loop used 256; we fix to `length`
    sort_desc(b, length);
    for (int i = 0; i < length; ++i) {
        if (a[i] != b[i]) {
            std::cout << a[i] << " " << b[i] << std::endl;
            print_two_lists(a, b, length);
            throw std::exception();
        }
    }
    return true;
}

void print_two_lists(int a[], int b[], int length) {
    sort_desc(a, length);
    sort_desc(b, length);
    for (int i = 0; i < length; ++i) std::cout << a[i] << " : " << b[i] << std::endl;
}

}