#pragma once
#include "Common.hpp"
#include "Board.hpp"
#include "Movegen.hpp"
#include "Eval.hpp"
#include "TT.hpp"
#include "Engine.hpp"

namespace bbc{
    // MVV LVA table
    // Assign capture weights : PawnXQueen > QueenXPawn
    inline constexpr int MVV_LVA[12][12] = {
        {60, 61, 62, 63, 64, 65, 60, 61, 62, 63, 64, 65}, 
        {50, 51, 52, 53, 54, 55, 50, 51, 52, 53, 54, 55}, 
        {40, 41, 42, 43, 44, 45, 40, 41, 42, 43, 44, 45}, 
        {30, 31, 32, 33, 34, 35, 30, 31, 32, 33, 34, 35},
        {20, 21, 22, 23, 24, 25, 20, 21, 22, 23, 24, 25}, 
        {10, 11, 12, 13, 14, 15, 10, 11, 12, 13, 14, 15},
        {60, 61, 62, 63, 64, 65, 60, 61, 62, 63, 64, 65}, 
        {50, 51, 52, 53, 54, 55, 50, 51, 52, 53, 54, 55}, 
        {40, 41, 42, 43, 44, 45, 40, 41, 42, 43, 44, 45}, 
        {30, 31, 32, 33, 34, 35, 30, 31, 32, 33, 34, 35},
        {20, 21, 22, 23, 24, 25, 20, 21, 22, 23, 24, 25}, 
        {10, 11, 12, 13, 14, 15, 10, 11, 12, 13, 14, 15},
    };

    void storeKillerMove(int move, int ply, SearchContext& sc);

    // assign score weightings
    inline constexpr int TT_SCORE = 10000;
    inline constexpr int CAPTURE_SCORE = 100;
    inline constexpr int KILLER_SCORE = 10;
    inline constexpr int OTHER_SCORE = 1;

    // score move before we sort
    void score_moves(MoveList& move_list, move_utility scores[256], int TT_move, const Board& board, SearchContext& sc);

    void sort_moves(MoveList& move_list, int TT_move, const Board& board, TranspositionTable& tt, SearchContext& sc);
}