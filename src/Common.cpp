# include "Common.hpp"
# include "Board.hpp"

namespace bbc {
    Board board;  // the one and only definition

    // bind the globals to the Board's members
    U64       (&bitboards)[12] = board.bitboards;
    U64       (&occupancies)[3] = board.occupancies;
    int       &side      = board.side;
    int       &enpassant = board.enpassant;
    int       &castle    = board.castle;
    int       &ply       = board.ply;


    
    void parse_fen(const char *fen) { board.parseFEN(fen); }
    void print_board() { board.print(); }
}