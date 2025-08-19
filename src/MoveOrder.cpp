#include "MoveOrder.hpp"
#include <algorithm>

namespace bbc{
    // store killer moves to a stack
    void storeKillerMove(int move, int ply, SearchContext& sc){
        // exit capture moves
        if(get_move_capture(move)) return;

        // exit if move exists alr
        if(move == sc.killerMoves[ply][0]) return;

        // shift moves FILO
        sc.killerMoves[ply][1] = sc.killerMoves[ply][0];
        sc.killerMoves[ply][0] = move;
    }


    // score moves to be sorted
    void score_moves(MoveList& move_list,  move_utility scores[256], int TT_move, const Board& board, SearchContext& sc){
        int move_cnt = move_list.count;
        const auto& ply = board.ply;
        const auto& killerMoves = sc.killerMoves;

        for(int i = 0; i < move_cnt; i++){
            int move = move_list.moves[i];
            scores[i].move = move;
            
            // 1: TT moves
            if(move==TT_move){
                scores[i].utility = TT_SCORE;
            }
            // 2: Capture moves
            else if(get_move_capture(move)){
                // Get mvv lva score
                int piece = get_move_piece(move);
                int target_sq = get_move_target(move);

                int victim = board.piece_at[target_sq];
                if(victim==no_piece){ // enpassant
                    victim = board.side==white? p : P;
                }

                scores[i].utility = MVV_LVA[11-victim][11-piece] * CAPTURE_SCORE;
            }
            // 3: Killer moves
            else if(move==killerMoves[ply][0]){
                scores[i].utility = 2 * KILLER_SCORE;
            }
            else if(move==killerMoves[ply][1]){
                scores[i].utility = 1 * KILLER_SCORE;
            }
            // 4: Non captures
            else{
                scores[i].utility = OTHER_SCORE;
            }
        }
    }

    //***************************
    //        SORT MOVES  *** MAY CONVERT TO MOVEPICKER **** 
    //**************************/
    void sort_moves(MoveList& move_list, int TT_move, const Board& board, TranspositionTable& tt, SearchContext& sc){
        const int num_moves = move_list.count;

        // 1) Score moves
        move_utility scores[256];
        score_moves(move_list, scores, TT_move, board, sc);
        
        // 2a) Insertion sort for small n (descending)
        if(num_moves <= 32){
            for(int i = 1; i < num_moves; i++){
                int score = scores[i].utility;
                int j = i;
                while(j>0 && score > scores[j-1].utility){
                    std::swap(scores[j], scores[j-1]);
                    j--;
                }
            }
        }
        // 2b) (quick) sort for larger n
        else{
            std::sort(scores, scores + num_moves, [](const move_utility& x, const move_utility& y){
                return x.utility > y.utility;
            });
        }

        // 3) update move_list
        for(int i = 0; i < num_moves; i++){
            move_list.moves[i] = scores[i].move;
        }

    }


}