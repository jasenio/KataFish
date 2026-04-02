#include "Common.hpp"
#include "Board.hpp"
#include "Attacks.hpp"
#include "Move.hpp"
#include "Movegen.hpp"
#include "Eval.hpp"
#include "Perft.hpp"
#include "TT.hpp"
#include "MoveOrder.hpp"
#include "Search.hpp"
#include "UCI.hpp"
#include "Engine.hpp"
#include "nn.hpp"
#include "nnue.hpp"

namespace bbc{

/**********************************\
 ==================================
 
              Init all
 
 ==================================
\**********************************/

// init all variables
void init_all()
{
    
    // init attack pieces
    init_attacks();

    // init zobrist table
    init_zobrist_table();
    
    // init magic numbers
    //init_magic_numbers();

    // init NNUE
    nnue::init("src/nn-eba324f53044.nnue");
}

/**********************************\
 ==================================
 
             Main driver
 
 ==================================
\**********************************/

int engine_main(int argc, char* argv[])
{

    // init all
    init_all();

    // init engine variables {board, table, s_context}
    Board board;
    board.parse_fen(start_position);
    
    int score = nnue::evaluate(board);
    std::printf("NNUE eval: %d\n", score);
    
    board.print_board();

    // engine context
    TranspositionTable tt;
    SearchContext sc;
    TimeContext tc;

    // call uci
    uci_loop(board, tc, tt, sc);

    return 0;
}

}

int main(int argc, char* argv[]) {
    bbc::engine_main(argc, argv);
    return 0;
}
