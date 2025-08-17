// ========================= movegen.cpp =========================
#include "Movegen.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>

namespace bbc {

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
    StateInfo st;
    for (int i = 0; i < list.count; ++i) {
        Board snap; copy_board(snap, board);
        if (make_move(list.moves[i], all_moves, board, st)) {
            restore_copy(snap, board);
            return true;
        }
        // if make_move failed it already restored
    }
    return false;
}

// -----------------------------
// Make move (with legality filter)
// -----------------------------
int make_move(int move, int move_flag, Board& board, StateInfo& st) {
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;
    auto& ply = board.ply;
    auto& piece_at =board.piece_at;

    if (move_flag != all_moves) {
        if (!get_move_capture(move)) return 0; // only accept captures
        // fallthrough to play it
    }

    // Preserve board state
    // Board copy; save_board(copy, board);

    // Fields
    int source_square   = get_move_source(move);
    int target_square   = get_move_target(move);
    int piece           = get_move_piece(move);
    int promoted_piece  = get_move_promoted(move);
    bool capture        = get_move_capture(move);
    bool double_push    = get_move_double(move);
    bool enpass         = get_move_enpassant(move);
    bool castl          = get_move_castling(move);

    // Remember State Info
    st.old_castle = board.castle;
    st.old_ep     = board.enpassant;
    st.old_ply    = board.ply;

    if (enpass) {
        st.captured = (side == white)? p : P;
        st.cap_sq   = (side == white)? target_square + 8 : target_square - 8;
    } else if (capture) {
        // read target piece type
        st.captured = piece_at[target_square];
        st.cap_sq   = target_square;
    } else {
        st.captured = no_piece;
        st.cap_sq   = no_sq;
    }

    // Move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);
    piece_at[source_square] = no_piece;
    piece_at[target_square] = piece;

    // Captures
    if (capture && !enpass) {
        if (st.captured != no_piece) pop_bit(bitboards[st.captured], target_square);
    }

    // Promotion
    if (promoted_piece) {
        pop_bit(bitboards[(side == white) ? P : p], target_square);
        set_bit(bitboards[promoted_piece], target_square);
        piece_at[target_square] = promoted_piece;
    }

    // En-passant capture
    // may be wrong ???                                        C   H E CK ****
    if (enpass) {
        if(side == white) pop_bit(bitboards[p], target_square + 8), piece_at[target_square+8]=no_piece;
        else pop_bit(bitboards[P], target_square - 8), piece_at[target_square-8]=no_piece;
    }

    // En-passant square
    enpassant = no_sq;
    if (double_push) {
        enpassant = (side == white) ? (target_square + 8) : (target_square - 8);
    }

    // Castling rook move
    if (castl) {
        switch (target_square) {
            case g1: pop_bit(bitboards[R], h1); set_bit(bitboards[R], f1); piece_at[h1]=no_piece; piece_at[f1] = R; break;
            case c1: pop_bit(bitboards[R], a1); set_bit(bitboards[R], d1); piece_at[a1]=no_piece; piece_at[d1] = R; break;
            case g8: pop_bit(bitboards[r], h8); set_bit(bitboards[r], f8); piece_at[h8]=no_piece; piece_at[f8] = r; break;
            case c8: pop_bit(bitboards[r], a8); set_bit(bitboards[r], d8); piece_at[a8]=no_piece; piece_at[d8] = r; break;
            default: break;
        }
    }

    // Update castling rights
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];

    // Rebuild occupancies
    const int us   = side;          // mover before flip
    const int them = us ^ 1;

    const U64 fromBB = 1ULL << source_square;
    const U64 toBB   = 1ULL << target_square;

    // Move our piece: from -> to
    occupancies[us] ^= (fromBB ^ toBB);

    // Remove captured piece from opponent occupancy
    if (st.captured != no_piece) {
        occupancies[them] ^= (1ULL << st.cap_sq); // EP uses st.cap_sq
    }

    // Rook movement affects our occupancy on castles
    if (castl) {
        int rook_from, rook_to;
        if (side == white) {
            if (target_square == g1)      { rook_from = h1; rook_to = f1; }
            else /* c1 */                 { rook_from = a1; rook_to = d1; }
        } else {
            if (target_square == g8)      { rook_from = h8; rook_to = f8; }
            else /* c8 */                 { rook_from = a8; rook_to = d8; }
        }
        occupancies[us] ^= ((1ULL << rook_from) ^ (1ULL << rook_to));
    }

    // Recompute 'both'
    occupancies[both] = occupancies[white] | occupancies[black];

    // Side to move
    side ^= 1; ++ply;

    // Legality: own king may not be in check
    int king_sq = (side == white) ? __builtin_ctzll(bitboards[k])
                                  : __builtin_ctzll(bitboards[K]);

    if (is_square_attacked(board, king_sq, side)) {
        restore_board(board, st, move);
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
