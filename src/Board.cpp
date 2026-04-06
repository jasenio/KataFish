
#include <iostream>
#include <cstring>        
#include "Board.hpp"

namespace bbc {
    // DEBUG
    int g_refreshes = 0;
    int g_updates = 0;
    int g_evals = 0;

    // constructor
    Board::Board() :
        side(white),
        enpassant(no_sq),
        castle(0),
        ply(0),
        fifty(0),
        hash(0),
        rep_len(0),
        rep_start(0),
        use_nnue(true),
        nnue_ply(0)
    {}
    
    /* ---------- parseFEN ---------- */
    void Board::parse_fen(const char* fen)
    {
        // reset board position (bitboards)
        std::memset(this->bitboards, 0ULL, sizeof(this->bitboards));
        
        // reset occupancies (bitboards)
        std::memset(this->occupancies, 0ULL, sizeof(this->occupancies));

        // reset piece memory
        std::memset(this->piece_at, no_piece, sizeof(this->piece_at));
        
        // reset game state variables
        this->side = 0;
        this->enpassant = no_sq;
        this->castle = 0;
        
        // loop over board ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop over board files
            for (int file = 0; file < 8; file++)
            {
                // init current square
                int square = rank * 8 + file;
                
                // match ascii pieces within FEN string
                if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
                {
                    // init piece type
                    int piece = char_to_piece(*fen);
                    
                    // set piece on corresponding bitboard
                    set_bit(this->bitboards[piece], square);
                    this->piece_at[square] = piece;

                    if(piece==K) king_sq[white] = square;
                    else if(piece==k) king_sq[black] = square;
                    
                    // increment pointer to FEN string
                    fen++;
                }
                
                // match empty square numbers within FEN string
                if (*fen >= '0' && *fen <= '9')
                {
                    // init offset (convert char 0 to int 0)
                    int offset = *fen - '0';
                    
                    // define piece variable
                    int piece = no_piece;
                    
                    // loop over all piece bitboards
                    for (int bb_piece = P; bb_piece <= k; bb_piece++)
                    {
                        // if there is a piece on current square
                        if (get_bit(this->bitboards[bb_piece], square))
                            // get piece code
                            piece = bb_piece;
                    }
                    
                    // on empty current square
                    if (piece == -1)
                        // decrement file
                        file--;
                    
                    // adjust file counter
                    file += offset;
                    
                    // increment pointer to FEN string
                    fen++;
                }
                
                // match rank separator
                if (*fen == '/')
                    // increment pointer to FEN string
                    fen++;
            }
        }
        
        // got to parsing this->side to move (increment pointer to FEN string)
        fen++;
        
        // parse this->side to move
        (*fen == 'w') ? (this->side = white) : (this->side = black);
        
        // go to parsing castling rights
        fen += 2;
        
        // parse castling rights
        while (*fen != ' ')
        {
            switch (*fen)
            {
                case 'K': this->castle |= wk; break;
                case 'Q': this->castle |= wq; break;
                case 'k': this->castle |= bk; break;
                case 'q': this->castle |= bq; break;
                case '-': break;
            }

            // increment pointer to FEN string
            fen++;
        }
        
        // got to parsing enpassant square (increment pointer to FEN string)
        fen++;
        
        // parse enpassant square
        if (*fen != '-')
        {
            // parse enpassant file & rank
            int file = fen[0] - 'a';
            int rank = 8 - (fen[1] - '0');
            
            // init enpassant square
            this->enpassant = rank * 8 + file;
        }
        
        // no enpassant square
        else
            this->enpassant = no_sq;
        
        // loop over white pieces bitboards
        for (int piece = P; piece <= K; piece++)
            // populate white occupancy bitboard
            this->occupancies[white] |= this->bitboards[piece];
        
        // loop over black pieces bitboards
        for (int piece = p; piece <= k; piece++)
            // populate white occupancy bitboard
            this->occupancies[black] |= this->bitboards[piece];
        
        // init all occupancies
        this->occupancies[both] |= this->occupancies[white];
        this->occupancies[both] |= this->occupancies[black];

        // set hash
        calc_hash();

        // store position in game history
        this->rep_len = 0;
        this->rep_start = 1;
        this->rep_keys[this->rep_len++] = this->hash;

        // fifty move rule
        this->fifty = 0;

        // init NNUE data
        this->nnue_ply = 0;

        auto& nn = this->nnue_stack[0];

        // Clear dirty info
        nn.dirtyPiece.dirtyNum = 0;
        for (int i = 0; i < 3; ++i) {
            nn.dirtyPiece.pc[i]   = blank;
            nn.dirtyPiece.from[i] = no_sq;
            nn.dirtyPiece.to[i]   = no_sq;
        }

        nn.accumulator.computedAccumulation = false;
    }

    /* ---------- print ---------- */
    void Board::print_board() const
    {
        // print offset
        printf("\n");

        // loop over board ranks
        for (int rank = 0; rank < 8; rank++)
        {
            // loop ober board files
            for (int file = 0; file < 8; file++)
            {
                // init square
                int square = rank * 8 + file;
                
                // print ranks
                if (!file)
                    printf("  %d ", 8 - rank);
                
                // define piece variable
                int piece = -1;
                
                // loop over all piece bitboards
                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    // if there is a piece on current square
                    if (get_bit(bitboards[bb_piece], square))
                        // get piece code
                        piece = bb_piece;
                }
                
                // print different piece set depending on OS
                #ifdef WIN64
                    printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
                #else
                    printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
                #endif
            }
            
            // print new line every rank
            printf("\n");
        }
        
        // print board files
        printf("\n     a b c d e f g h\n\n");
        
        // print this->side to move
        printf("     side:     %s\n", !side ? "white" : "black");
        
        // print enpassant square
        printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_coordinates[enpassant] : "no");
        
        // print castling rights
        printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                            (castle & wq) ? 'Q' : '-',
                                            (castle & bk) ? 'k' : '-',
                                            (castle & bq) ? 'q' : '-');

    }

    
    // get hash
    void Board::calc_hash() {
        this->hash = 0ULL;
        const auto& bitboards = this->bitboards;
        const auto& side = this->side;
        const auto& castle = this->castle;
        const auto& enpassant = this->enpassant;

        // calc hash with all piece bitboards
        for(int piece = P; piece <= k; piece++){
            uint64_t bitboard = bitboards[piece];

            // hash using xor of ALL piece positions
            while(bitboard){
                int square = __builtin_ctzll(bitboard);

                xor_piece(piece, square);
                pop_bit(bitboard, square);
            }
        }

        // calc hash with side
        this->hash = side==white? hash : hash ^ random_side;

        // calc hash with castling (0, castle = no castling rights -> castling rights)
        this->hash ^= random_castling[castle];

        // calc hash with enpassant values
        xor_ep(enpassant);
    }

    // ---- Edit helpers ----
    void Board::place(int piece, int sq){
        
    }

    void Board::remove(int piece, int sq){

    }

    // ---- Hash  helpers ----
    void Board::xor_piece(int piece, int sq) {
        this->hash ^= random_pieces[piece * 64 + sq];
    }


    void Board::xor_castling(int old_cr, int new_cr) {
        if (old_cr != new_cr) this->hash ^= (random_castling[old_cr] ^ random_castling[new_cr]);
    }

    void Board::xor_ep(int ep) {
        if (ep >= 16 && ep <= 23) this->hash ^= random_file[ep - 16];
        else if (ep >= 40 && ep <= 47) this->hash ^= random_file[ep - 40];
    }


    // random pieces
    U64 random_pieces[768];
    U64 random_side;
    U64 random_castling[16];
    U64 random_file[8];

    // init zobrist table
    void init_zobrist_table(){
        for(int i = 0; i < 768; i++){
            random_pieces[i] = get_random_U64_number();
        }
        random_side = get_random_U64_number();
        for(int i = 0; i < 16; i++){
            random_castling[i] = get_random_U64_number();
        }
        for(int i = 0; i < 8; i++){
            random_file[i] = get_random_U64_number();
        }
    }
} // namespace bbc
