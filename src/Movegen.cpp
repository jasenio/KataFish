// ========================= movegen.cpp =========================
#include "Movegen.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>

namespace bbc {

// -----------------------------
// Local (translation-unit) constants
// -----------------------------
// Castling rights update by square; only used here -> keep TU-local.
static const int castling_rights[64] = {
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
// Add move
// -----------------------------
void add_move(MoveList& list, int move) {
    list.moves[list.count++] = move;
}

// -----------------------------
// Attack queries
// -----------------------------
int is_square_attacked(const Board& board, int square, int side_) {
    const auto& bitboards = board.bitboards;
    const auto& occupancies = board.occupancies;
    // pawnss
    if ((side_ == white) && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if ((side_ == black) && (pawn_attacks[white][square] & bitboards[p])) return 1;

    // knights
    if (knight_attacks[square] & ((side_ == white) ? bitboards[N] : bitboards[n])) return 1;

    // bishops/rooks/queens (sliders)
    if (get_bishop_attacks(square, occupancies[both]) & ((side_ == white) ? bitboards[B] : bitboards[b])) return 1;
    if (get_rook_attacks  (square, occupancies[both]) & ((side_ == white) ? bitboards[R] : bitboards[r])) return 1;
    if (get_queen_attacks (square, occupancies[both]) & ((side_ == white) ? bitboards[Q] : bitboards[q])) return 1;

    // kings
    if (king_attacks[square] & ((side_ == white) ? bitboards[K] : bitboards[k])) return 1;

    return 0;
}

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
inline bool in_check_now(const Board& board) {
    const auto& side = board.side;
    const auto& bitboards = board.bitboards;
    int myKing = (side == white)
        ? __builtin_ctzll(bitboards[K])
        : __builtin_ctzll(bitboards[k]);
    int opp = (side == white) ? black : white;
    return is_square_attacked(board, myKing, opp);
}

inline bool has_legal_move(Board& board) {
    MoveList list; 
    generate_moves(list, board);
    for (int i = 0; i < list.count; ++i) {
        Board snap; save_board(snap, board);
        if (make_move(list.moves[i], all_moves, board)) {
            restore_board(snap, board);
            return true;
        }
        // if make_move failed it already restored
    }
    return false;
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

// -----------------------------
// Make move (with legality filter)
// -----------------------------
int make_move(int move, int move_flag, Board& board) {
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;
    auto& ply = board.ply;


    if (move_flag != all_moves) {
        if (!get_move_capture(move)) return 0; // only accept captures
        // fallthrough to play it
    }

    // Preserve board state
    Board copy; save_board(copy, board);

    // Fields
    int source_square   = get_move_source(move);
    int target_square   = get_move_target(move);
    int piece           = get_move_piece(move);
    int promoted_piece  = get_move_promoted(move);
    bool capture        = get_move_capture(move);
    bool double_push    = get_move_double(move);
    bool enpass         = get_move_enpassant(move);
    bool castl          = get_move_castling(move);

    // Move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);

    // Captures
    if (capture) {
        int start_piece = (side == white) ? p : P;
        int end_piece   = (side == white) ? k : K;
        for (int bbp = start_piece; bbp <= end_piece; ++bbp) {
            if (get_bit(bitboards[bbp], target_square)) { pop_bit(bitboards[bbp], target_square); break; }
        }
    }

    // Promotion
    if (promoted_piece) {
        pop_bit(bitboards[(side == white) ? P : p], target_square);
        set_bit(bitboards[promoted_piece], target_square);
    }

    // En-passant capture
    // may be wrong ???                                        C   H E CK ****
    if (enpass) {
        (side == white) ? pop_bit(bitboards[p], target_square + 8)
                        : pop_bit(bitboards[P], target_square - 8);
    }

    // En-passant square
    enpassant = no_sq;
    if (double_push) {
        enpassant = (side == white) ? (target_square + 8) : (target_square - 8);
    }

    // Castling rook move
    if (castl) {
        switch (target_square) {
            case g1: pop_bit(bitboards[R], h1); set_bit(bitboards[R], f1); break;
            case c1: pop_bit(bitboards[R], a1); set_bit(bitboards[R], d1); break;
            case g8: pop_bit(bitboards[r], h8); set_bit(bitboards[r], f8); break;
            case c8: pop_bit(bitboards[r], a8); set_bit(bitboards[r], d8); break;
            default: break;
        }
    }

    // Update castling rights
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];

    // Rebuild occupancies
    std::memset(occupancies, 0, sizeof(occupancies));
    for (int bbp = P; bbp <= K; ++bbp) occupancies[white] |= bitboards[bbp];
    for (int bbp = p; bbp <= k; ++bbp) occupancies[black] |= bitboards[bbp];
    occupancies[both] = occupancies[white] | occupancies[black];

    // Side to move
    side ^= 1; ++ply;

    // Legality: own king may not be in check
    int king_sq = (side == white) ? __builtin_ctzll(bitboards[k])
                                  : __builtin_ctzll(bitboards[K]);

    if (is_square_attacked(board, king_sq, side)) {
        restore_board(copy, board);
        return 0;
    }

    return 1;
}

// -----------------------------
// Generate pseudo-legal moves
// -----------------------------


// CHANGE TO STATE INFOOO AOSDOAOSD AS
void generate_moves(MoveList& list, Board& board) {
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
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        } else {
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
                if (castle & wk) {
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
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        } else {
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
                    if (!get_bit((side == white ? occupancies[black] : occupancies[white]), target_square))
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    else
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
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
                    if (!get_bit((side == white ? occupancies[black] : occupancies[white]), target_square))
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    else
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
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
                    if (!get_bit((side == white ? occupancies[black] : occupancies[white]), target_square))
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    else
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
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
                    if (!get_bit((side == white ? occupancies[black] : occupancies[white]), target_square))
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    else
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
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
                    if (!get_bit((side == white ? occupancies[black] : occupancies[white]), target_square))
                        add_move(list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    else
                        add_move(list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    pop_bit(attacks, target_square);
                }
                pop_bit(bitboard, source_square);
            }
        }
    }
}

} // namespace bbc
