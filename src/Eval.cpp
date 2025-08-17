#include "Eval.hpp"

namespace bbc{

int eval(const Board& board){
    const auto& bitboards = board.bitboards;
    // BASIC MATERIAL SCORE, not middle/endgame taken into account
    int countPieces[12] = {};

    uint64_t white_pawns = bitboards[P];
    uint64_t black_pawns = bitboards[p];
    uint64_t white_knights = bitboards[N];
    uint64_t black_knights = bitboards[n];
    uint64_t white_bishops = bitboards[B];
    uint64_t black_bishops = bitboards[b];
    uint64_t white_rooks = bitboards[R];
    uint64_t black_rooks = bitboards[r];
    uint64_t white_queens = bitboards[Q];
    uint64_t black_queens = bitboards[q];
    uint64_t white_king = bitboards[K];
    uint64_t black_king = bitboards[k];

    
    // PIECE SQUARE TABLE EVALUATION
    int utility_table = 0;

    // calc pawns
    while (white_pawns) {
        countPieces[P]++;
        int pawn_square = __builtin_ctzll(white_pawns);
        utility_table += pawn_table[pawn_square];
        pop_bit(white_pawns, pawn_square);
    }

    while (black_pawns) {
        countPieces[p]++;
        int pawn_square = __builtin_ctzll(black_pawns);
        utility_table -= pawn_table[63 - pawn_square];
        pop_bit(black_pawns, pawn_square);
    }

    // calc knights
    while (white_knights) {
        countPieces[N]++;
        int knight_square = __builtin_ctzll(white_knights);
        utility_table += knight_table[knight_square];
        pop_bit(white_knights, knight_square);
    }

    while (black_knights) {
        countPieces[n]++;
        int knight_square = __builtin_ctzll(black_knights);
        utility_table -= knight_table[63 - knight_square];
        pop_bit(black_knights, knight_square);
    }

    // calc bishops
    while (white_bishops) {
        countPieces[B]++;
        int bishop_square = __builtin_ctzll(white_bishops);
        utility_table += bishop_table[bishop_square];
        pop_bit(white_bishops, bishop_square);
    }

    while (black_bishops) {
        countPieces[b]++;
        int bishop_square = __builtin_ctzll(black_bishops);
        utility_table -= bishop_table[63 - bishop_square];
        pop_bit(black_bishops, bishop_square);
    }

    // calc rooks
    while (white_rooks) {
        countPieces[R]++;
        int rook_square = __builtin_ctzll(white_rooks);
        utility_table += rook_table[rook_square];
        pop_bit(white_rooks, rook_square);
    }

    while (black_rooks) {
        countPieces[r]++;
        int rook_square = __builtin_ctzll(black_rooks);
        utility_table -= rook_table[63 - rook_square];
        pop_bit(black_rooks, rook_square);
    }

    // calc queens
    while (white_queens) {
        countPieces[Q]++;
        int queen_square = __builtin_ctzll(white_queens);
        utility_table += queen_table[queen_square];
        pop_bit(white_queens, queen_square);
    }

    while (black_queens) {
        countPieces[q]++;
        int queen_square = __builtin_ctzll(black_queens);
        utility_table -= queen_table[63 - queen_square];
        pop_bit(black_queens, queen_square);
    }

    // calculate score based on centipawns
    int utility_material = 0;
    // count material
    utility_material += 100 * (countPieces[P] - countPieces[p]);
    utility_material += 320 * (countPieces[N] - countPieces[n]);
    utility_material += 330 * (countPieces[B] - countPieces[b]);
    utility_material += 500 * (countPieces[R] - countPieces[r]);
    utility_material += 900 * (countPieces[Q] - countPieces[q]);

    // end game starts when less than 7 major/minor pieces left
    bool end_game = (countPieces[N] + countPieces[n] +
                    countPieces[B] + countPieces[b] +
                    countPieces[R] + countPieces[r] +
                    countPieces[Q] + countPieces[q] <= 6);

    // calc kings depending on endgame
    while (white_king) {
        countPieces[K]++;
        int king_square = __builtin_ctzll(white_king);
        utility_table += end_game ? king_end_table[king_square] : king_middle_table[king_square];
        pop_bit(white_king, king_square);
    }

    while (black_king) {
        countPieces[k]++;
        int king_square = __builtin_ctzll(black_king);
        utility_table -= end_game ? king_end_table[63 - king_square] : king_middle_table[63 - king_square];
        pop_bit(black_king, king_square);
    }

    utility_material += 20000 * (countPieces[K] - countPieces[k]);

    return utility_material + utility_table;
}

}