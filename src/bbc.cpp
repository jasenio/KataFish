/**********************************\
 ==================================
           
             Didactic
       BITBOARD CHESS ENGINE     
       
                by
                
         Code Monkey King

  **SEARCH AND EVALUATION ON LINES 2225**

                by

              jasenio
 
 ==================================
\**********************************/

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

    // Create engine variables {board, table, s_context}
    Board board;
    board.parse_fen(start_position);
    board.print_board();
    TranspositionTable tt;
    SearchContext sc;
    TimeContext tc;

    // allocate 3 minutes + 3 seconds
    tc.ms_left = 3 * 60 * 1000;
    tc.ms_inc = 3 * 1000;

    // call uci
    uci_loop(board, tc, tt, sc);

    return 0;
}

}

int main(int argc, char* argv[]) {
    bbc::engine_main(argc, argv);
    return 0;
}
