#pragma once
#include "Common.hpp"
#include "Board.hpp"
#include "Movegen.hpp"
#include "Eval.hpp"
#include "TT.hpp"
#include "MoveOrder.hpp"
#include "Engine.hpp"


/**********************************\
 ==================================
 
               Search

  AIMA CODE IMPLEMENTATION HERE
 
 ==================================
\**********************************/

namespace bbc{

    // q search at terminal depths (d=0)
    int qsearch(int alpha, int beta, Board& board, TranspositionTable& tt, SearchContext& sc);

    // search at given depth (d)
    move_utility negamax(int alpha, int beta, int depth, Board& board, TranspositionTable& tt, SearchContext& sc);

    // search in consecutive depths (1->2->...d)
    move_utility iterative_deepening(int depth, TimeContext& tc, Board& board, TranspositionTable& tt, SearchContext& sc);
}