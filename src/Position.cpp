#include "Position.hpp"

namespace bbc{

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
    st.old_king_sq[white] = board.king_sq[white];
    st.old_king_sq[black] = board.king_sq[black];   
    // hash
    st.old_hash = board.hash;
    // pos history
    st.old_rep_len   = board.rep_len;
    st.old_rep_start = board.rep_start;

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
    // 0) clear enpassant hash
    if(enpassant!=no_sq) board.xor_ep(enpassant);
    enpassant = no_sq;

    // 1) Move piece
    pop_bit(bitboards[piece], source_square);
    set_bit(bitboards[piece], target_square);
    piece_at[source_square] = no_piece;
    piece_at[target_square] = piece;
    board.xor_piece(piece, source_square);

    // update king_sq
    if(piece==K) board.king_sq[white] = target_square;
    else if(piece==k) board.king_sq[black] = target_square;

    // 2) Captures/Enpassant
    if (capture) {
        // 2a) En-passant capture ***
        if (enpass) {
            if(side == white) {
                pop_bit(bitboards[p], target_square + 8);
                piece_at[target_square+8]=no_piece;
                board.xor_piece(st.captured, st.cap_sq);
            }
            else{ 
                pop_bit(bitboards[P], target_square - 8);
                piece_at[target_square-8]=no_piece;
                board.xor_piece(st.captured, st.cap_sq);
            }
        }
        else if (st.captured != no_piece){
            pop_bit(bitboards[st.captured], target_square);
            board.xor_piece(st.captured, target_square);
        } 
    }

    // 3) Promotion
    if (promoted_piece) {
        pop_bit(bitboards[(side == white) ? P : p], target_square);
        set_bit(bitboards[promoted_piece], target_square);
        piece_at[target_square] = promoted_piece;
        board.xor_piece(promoted_piece, target_square);
    }
    else board.xor_piece(piece, target_square);

    // 4) Double pawn push
    if (double_push) {
        enpassant = (side == white) ? (target_square + 8) : (target_square - 8);
        board.xor_ep(enpassant);
    }
    else enpassant = no_sq;

    // 5) Castles
    if (castl) {
        switch (target_square) {
            case g1: 
                pop_bit(bitboards[R], h1); board.xor_piece(R, h1);
                set_bit(bitboards[R], f1); board.xor_piece(R, f1);
                piece_at[h1]=no_piece; 
                piece_at[f1] = R; break;
            case c1: 
                pop_bit(bitboards[R], a1); board.xor_piece(R, a1);
                set_bit(bitboards[R], d1); board.xor_piece(R, d1);
                piece_at[a1]=no_piece; 
                piece_at[d1] = R; break;
            case g8: 
                pop_bit(bitboards[r], h8); board.xor_piece(r, h8);
                set_bit(bitboards[r], f8); board.xor_piece(r, f8);
                piece_at[h8]=no_piece; 
                piece_at[f8] = r; break;
            case c8:
                pop_bit(bitboards[r], a8); board.xor_piece(r, a8);
                set_bit(bitboards[r], d8); board.xor_piece(r, d8);
                piece_at[a8]=no_piece; 
                piece_at[d8] = r; break;
            default: break;
        }
    }

    // Update castling rights
    castle &= castling_rights[source_square];
    castle &= castling_rights[target_square];
    board.xor_castling(st.old_castle, castle);

    // 6) Rebuild occupancies
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

    // 7) Side to move
    side ^= 1; ++ply;
    board.hash ^= random_side;

    // 8) Board history
    if(capture || piece==P || piece == p) board.rep_start = board.rep_len;
    board.rep_keys[board.rep_len++] = board.hash;

    // Legality: own king may not be in check
    int king_sq = board.king_sq[side^1];

    if (is_square_attacked(board, king_sq, side)) {
        undo_move(board, st, move);
        return 0;
    }

    return 1;
}

void undo_move(Board& b, const StateInfo& st, const int move) {
        auto& bitboards   = b.bitboards;
        auto& occupancies = b.occupancies;
        auto& side = b.side;
        auto& piece_at = b.piece_at;

        int source_square   = get_move_source(move);
        int target_square   = get_move_target(move);
        int piece           = get_move_piece(move);
        int promoted_piece  = get_move_promoted(move);
        bool enpass         = get_move_enpassant(move);
        bool castl          = get_move_castling(move);

        // After make_move, b.side has already been flipped.
        // The mover’s color is therefore (b.side ^ 1).
        const bool moverIsWhite = ((side ^ 1) == white);

        // 1) Undo castling rook move (if any)
        if (castl) {
            switch (target_square) {
                case g1: pop_bit(bitboards[R], f1); set_bit(bitboards[R], h1); piece_at[h1]=R; piece_at[f1] = no_piece; break;
                case c1: pop_bit(bitboards[R], d1); set_bit(bitboards[R], a1); piece_at[a1]=R; piece_at[d1] = no_piece; break;
                case g8: pop_bit(bitboards[r], f8); set_bit(bitboards[r], h8); piece_at[h8]=r; piece_at[f8] = no_piece; break;
                case c8: pop_bit(bitboards[r], d8); set_bit(bitboards[r], a8); piece_at[a8]=r; piece_at[d8] = no_piece; break;
                default: break;
            }
        }

        // 2) Undo promotion OR normal piece move
        if (promoted_piece) {
            // Remove promoted piece from 'to', put pawn back on 'from'
            if (moverIsWhite) {
                pop_bit(bitboards[promoted_piece], target_square);
                set_bit(bitboards[P], source_square);
                piece_at[target_square] = no_piece;
                piece_at[source_square] = P;
            } else {
                pop_bit(bitboards[promoted_piece], target_square);
                set_bit(bitboards[p], source_square);
                piece_at[target_square] = no_piece;
                piece_at[source_square] = p;
            }
        } else {
            // Move the original mover back from 'to' to 'from'
            pop_bit(bitboards[piece], target_square);
            set_bit(bitboards[piece], source_square);
            piece_at[target_square] = no_piece;
            piece_at[source_square] = piece;
        }

        // 3) Restore captured piece (normal or EP)
        if (st.captured != -1) {
            set_bit(bitboards[st.captured], st.cap_sq);
            piece_at[st.cap_sq] = st.captured;
            if (enpass) {
                // If EP, the 'to' square was empty; nothing to clear there now.
                // We already moved the mover back above; just put pawn at cap_sq.
            }
        }

        // 4) Restore lightweight board fields
        b.castle    = st.old_castle;
        b.enpassant = st.old_ep;
        b.ply       = st.old_ply;
        b.king_sq[white]   = st.old_king_sq[white];
        b.king_sq[black] = st.old_king_sq[black];
        b.hash = st.old_hash;
        b.rep_len = st.old_rep_len;
        b.rep_start = st.old_rep_start;

        // 5) Rebuild occupancies (since your make_move rebuilds them)
        const int moverSide   = moverIsWhite ? white : black;
        const int victimSide  = moverIsWhite ? black : white;

        // The mover was moved back: to -> from
        const U64 moveMask = (1ULL << source_square) ^ (1ULL << target_square);
        occupancies[moverSide] ^= moveMask;

        // If there was a capture (normal or EP), we just restored the victim bit above.
        // So we need to flip that bit back on in the victim's occupancy as well.
        if (st.captured != -1) {
            occupancies[victimSide] ^= (1ULL << st.cap_sq);
        }

        // If this was a castle, we also moved the rook back; flip its squares.
        if (castl) {
            int rook_from, rook_to;
            if (moverIsWhite) {
                if (target_square == g1)      { rook_from = f1; rook_to = h1; } // undoing: f1 -> h1
                else /* target_square == c1 */{ rook_from = d1; rook_to = a1; } // undoing: d1 -> a1
            } else {
                if (target_square == g8)      { rook_from = f8; rook_to = h8; }
                else /* target_square == c8 */{ rook_from = d8; rook_to = a8; }
            }
            const U64 rookMask = (1ULL << rook_from) ^ (1ULL << rook_to);
            occupancies[moverSide] ^= rookMask;
        }

        // Recompute 'both' with one OR (cheap, and keeps it exact)
        occupancies[both] = occupancies[white] | occupancies[black];

        // 6) Flip side back to the original mover
        b.side ^= 1;
}

// Null moves
void make_null_move(Board& board, StateInfo& st){
    st.old_ep = board.enpassant;
    st.old_hash = board.hash;

    if (board.enpassant != no_sq) board.xor_ep(board.enpassant);
    board.enpassant = no_sq;
    
    board.side^= 1;
    board.hash ^= random_side;
}
void restore_null(Board& board, StateInfo& st){
    board.side^=1;
    board.enpassant = st.old_ep;
    board.hash      = st.old_hash;
}


}