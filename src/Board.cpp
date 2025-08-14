
#include <iostream>
#include <cstring>          // if the old code used C-string ops
#include "Common.hpp"
#include "Board.hpp"

namespace bbc {
    // constructor
    Board::Board(){
        side      = white;
        enpassant = no_sq;
        castle    = 0;
        ply       = 0;
    }
    /* ---------- parseFEN ---------- */
    void Board::parse_fen(const char* fen)
    {
            /*  👉  Paste the body of your old  parse_fen(const char*)  here.
            For every former global, switch to a member:

                bitboards[i]   →  this->bitboards[i]
                side           →  this->side
                enpassant      →  this->enpassant
                castle         →  this->castle
                occupancies[x] →  this->occupancies[x]

            Nothing else changes.  Keep helper calls (set_bit, etc.) as-is.
        */
        // reset board position (bitboards)
        std::memset(this->bitboards, 0ULL, sizeof(this->bitboards));
        
        // reset occupancies (bitboards)
        std::memset(this->occupancies, 0ULL, sizeof(this->occupancies));
        
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
                    
                    // increment pointer to FEN string
                    fen++;
                }
                
                // match empty square numbers within FEN string
                if (*fen >= '0' && *fen <= '9')
                {
                    // init offset (convert char 0 to int 0)
                    int offset = *fen - '0';
                    
                    // define piece variable
                    int piece = -1;
                    
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

} // namespace bbc
