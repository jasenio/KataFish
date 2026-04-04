// ========================= movegen.cpp =========================
#include "Movegen.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>


namespace bbc {

void print_attacked_squares(const Board& board, int side_) {
    std::printf("\n");
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            if (!file) std::printf("  %d ", 8 - rank);
            std::printf(" %d", is_square_attacked(board, sq, side_) ? 1 : 0);
        }
        std::printf("\n");
    }
    std::printf("\n     a b c d e f g h\n\n");
}


// checkmate helpers
bool in_check_now(const Board& board) {
    return is_square_attacked(board, board.king_sq[board.side], board.side^1);
}

bool has_legal_move(Board& board) {
    MoveList list; 
    generate_moves(list, board);
    StateInfo st;
    for (int i = 0; i < list.count; ++i) {
        if (make_move(list.moves[i], all_moves, board, st)) {
            // Proper undo that restores rep_len/rep_start as well
            undo_move(board, st, list.moves[i]);
            return true;
        }
        // if make_move failed it already restored
    }
    return false;
}

// -----------------------------
// Generate pseudo-legal moves
// -----------------------------

// CHANGE TO STATE INFOOO AOSDOAOSD AS
void generate_moves(MoveList& list, Board& board, bool quiet) {
    // quiet -> promotions + captures
    list.count = 0;
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;

    int source_square, target_square;
    U64 bitboard, attacks;

    for (int piece = P; piece <= k; ++piece) {
        bitboard = bitboards[piece];

        if (side == white) {
            // White pawns
            if (piece == P) {
                while (bitboard) {
                    source_square = __builtin_ctzll(bitboard);
                    target_square = source_square - 8;

                    // quiet pushes
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square)) {
                        // promotion
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        } 
                        else if (quiet){
                            add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                add_move(list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // captures
                    attacks = pawn_attacks[side][source_square] & occupancies[black];
                    while (attacks) {
                        target_square = __builtin_ctzll(attacks);
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                        } else {
                            add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target_square);
                    }

                    // en-passant
                    if (enpassant != no_sq) {
                        U64 ep = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        if (ep) {
                            int t = __builtin_ctzll(ep);
                            add_move(list, encode_move(source_square, t, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }

            // White castling (handled while scanning K)
            if (piece == K) {
                if ( castle & wk) {
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1)) {
                        if (!is_square_attacked(board, e1, black) && !is_square_attacked(board,f1, black))
                            add_move(list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                if (castle & wq) {
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1)) {
                        if (!is_square_attacked(board, e1, black) && !is_square_attacked(board, d1, black))
                            add_move(list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        } else {
            // Black pawns
            if (piece == p) {
                while (bitboard) {
                    source_square = __builtin_ctzll(bitboard);
                    target_square = source_square + 8;

                    // quiet pushes
                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square)) {
                        // promotion
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        } 
                        else if(quiet){
                            add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                add_move(list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                        }
                    }

                    // captures
                    attacks = pawn_attacks[side][source_square] & occupancies[white];
                    while (attacks) {
                        target_square = __builtin_ctzll(attacks);
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                        } else {
                            add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        pop_bit(attacks, target_square);
                    }

                    // en-passant
                    if (enpassant != no_sq) {
                        U64 ep = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        if (ep) {
                            int t = __builtin_ctzll(ep);
                            add_move(list, encode_move(source_square, t, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }

            // Black castling
            if (piece == k) {
                if (castle & bk) {
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8)) {
                        if (!is_square_attacked(board,  e8, white) && !is_square_attacked(board,  f8, white))
                            add_move(list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }
                if (castle & bq) {
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8)) {
                        if (!is_square_attacked(board,  e8, white) && !is_square_attacked(board,  d8, white))
                            add_move(list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }

        // Knights
        if ((side == white) ? piece == N : piece == n) {
            while (bitboard) {
                source_square = __builtin_ctzll(bitboard);
                attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                while (attacks) {
                    target_square = __builtin_ctzll(attacks);
                    bool isCap = get_bit((side == white ? occupancies[black] : occupancies[white]), target_square);
                    if (isCap) {
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    else if(quiet){ 
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }

        // Bishops
        if ((side == white) ? piece == B : piece == b) {
            while (bitboard) {
                source_square = __builtin_ctzll(bitboard);
                attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                while (attacks) {
                    target_square = __builtin_ctzll(attacks);
                    bool isCap = get_bit((side == white ? occupancies[black] : occupancies[white]), target_square);
                    if (isCap) {
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    else if(quiet){ 
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }

        // Rooks
        if ((side == white) ? piece == R : piece == r) {
            while (bitboard) {
                source_square = __builtin_ctzll(bitboard);
                attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                while (attacks) {
                    target_square = __builtin_ctzll(attacks);
                    bool isCap = get_bit((side == white ? occupancies[black] : occupancies[white]), target_square);
                    if (isCap) {
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    else if(quiet){ 
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }

        // Queens
        if ((side == white) ? piece == Q : piece == q) {
            while (bitboard) {
                source_square = __builtin_ctzll(bitboard);
                attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                while (attacks) {
                    target_square = __builtin_ctzll(attacks);
                    bool isCap = get_bit((side == white ? occupancies[black] : occupancies[white]), target_square);
                    if (isCap) {
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    else if(quiet){ 
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }

        // Kings (normal king steps)
        if ((side == white) ? piece == K : piece == k) {
            while (bitboard) {
                source_square = __builtin_ctzll(bitboard);
                attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                while (attacks) {
                    target_square = __builtin_ctzll(attacks);
                    bool isCap = get_bit((side == white ? occupancies[black] : occupancies[white]), target_square);
                    if (isCap) {
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    else if(quiet){ 
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }
    }
}

} // namespace bbc
