#include "Search.hpp"

namespace bbc{

// q search at d=0
int qsearch(int alpha, int beta, Board& board, TranspositionTable& tt, SearchContext& sc){
    bool check = in_check_now(board);
    // Stand-pat only if NOT in check
    if (!check) {
        int stand = eval(board);
        if (stand >= beta) return stand;            // fail-high
        if (stand > alpha) alpha = stand;           // raise alpha
    } else {
        // In check: no stand-pat; treat like one-ply extension of evasions
        // (i.e., we must consider *all* legal moves, not just captures)
    }

    MoveList ml;
    StateInfo st;

    if (!check) {
        generate_moves(ml, board, false);               // captures/promotions only
    } else {
        generate_moves(ml, board);                  // all legal moves (evasions)
    }

    // sort moves
    sort_moves(ml, 0, board, tt, sc);

    bool any = false;
    for (int i = 0; i < ml.count; ++i) {
        int move = ml.moves[i];
        
        if (!make_move(move, /*only_captures ok for speed*/ all_moves, board, st))
            continue;

        any = true;
        int score = -qsearch(-beta, -alpha, board, tt, sc);

        restore_board(board, st, move);

        if (score >= beta) return score;            // fail-high / cutoff
        if (score > alpha) alpha = score;           // best so far
    }

    if(check && !any){ // checkmate = in check + no legal moves
        return -MATE;
    }

    return alpha;
}

// from AIMA, game is preserved in global array bitboards[] instead of an input, copy and takeback mimic this operation
move_utility negamax(int alpha, int beta, int depth, Board& board, TranspositionTable& tt, SearchContext& sc) {
    poll_time(sc); // increment node and check time
    if(sc.stop) return {0, 0};

    // 1: Quiescence Search at terminal nodes
    if (depth == 0) {
        int s = qsearch(alpha, beta, board, tt, sc);   // qsearch must be negamax-compatible
        return {s, 0};
    }

    int alpha0 = alpha;

    // 2: TT probe
    TTEntry ent;
    bool probed = tt.probe(board.hash, ent, depth);
    if (probed && ent.depth >= depth) {
        if (ent.node_type == EXACT) {
            return {ent.value, ent.move};
        }
        if (ent.node_type == LOWER_BOUND) alpha = std::max(alpha, ent.value);
        else if (ent.node_type == UPPER_BOUND) beta = std::min(beta, ent.value);
        if (alpha >= beta) {
            return {ent.value, ent.move};
        }
    }

    // 3: Null move
    if(sc.null_enabled && alpha != beta -1){ // not a null branch
        if(depth >= 3 && board.ply >= 1){ // sufficient depth
            int check = in_check_now(board);
            if(!check){ // not in check
                U64 pieces = board.bitboards[Q] | board.bitboards[R] | board.bitboards[B] | board.bitboards[N] |
                             board.bitboards[q] | board.bitboards[r] | board.bitboards[b] | board.bitboards[n];
                if(pieces){ // pieces present
                    int static_eval = eval(board);
                    if(static_eval>=beta){ // sufficient strength to continue
                        StateInfo st;
                        make_null_move(board, st);
                        int R = 2 + depth/6;
                        move_utility score = negamax(-beta, -beta+1, depth-1-R, board, tt, sc);
                        restore_null(board, st);

                        if(-score.utility >= beta){ //success
                            return {beta, 0};
                        }
                    }
                    
                }
            }
        }
    }

    MoveList ml;
    generate_moves(ml, board);

    // 4: Sort better moves first
    sort_moves(ml, ent.move, board, tt, sc);

    // 5: Try making every legal move
    int bestScore = -INF;
    int bestMove  = 0;
    bool hasLegal = false;

    StateInfo st;
    for (int i = 0; i < ml.count; ++i) {
        int move = ml.moves[i];

        if (!make_move(move, all_moves, board, st)) continue;

        hasLegal = true;

        move_utility child = negamax(-beta, -alpha, depth - 1, board, tt, sc);
        if(sc.stop) return {0, 0}; // terminate on time

        int score = -child.utility;

        restore_board(board, st, move);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;

            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    storeKillerMove(move, board.ply, sc);
                    tt.store(board.hash, move, depth, bestScore, LOWER_BOUND);
                    return {bestScore, bestMove};
                }
            }
        }
    }

    // 6: Check for checkmate
    if (!hasLegal) {
        bool check = in_check_now(board);
        if (check)  return {-MATE+depth, 0}; // or -MATE + ply for mate distance
        return {0, 0};                  // stalemate
    }

    // 7: Update node type and store in tt
    int t =
        (bestScore <= alpha0) ? UPPER_BOUND :
        (bestScore >= beta)   ? LOWER_BOUND :
                               EXACT;

    tt.store(board.hash, bestMove, depth, bestScore, t);
    return {bestScore, bestMove};
}

// iterative deepening
move_utility iterative_deepening(int depth, TimeContext& tc, Board& board, TranspositionTable& tt, SearchContext& sc){
    int reached = 0;
    move_utility best;
    U64 prev_time = 0;
    for(int i = 1; i <= depth; i++){
        U64 cur_time = get_time_ms();

        move_utility cur_move = negamax(-INF, INF, i, board, tt, sc);
        if(sc.stop) break; // terminated early, don't use this

        U64 elapsed_time   = get_time_ms() - sc.start;     // since move start
        uint64_t depth_time  = get_time_ms() - cur_time;  

        best = cur_move;
        reached++;

        if(DEBUG) printf("Nodes: %lld Time: %lld\n", sc.nodes, get_time_ms()-sc.start); // print time taken for each depth

        // 1) Terminate on soft time
        if(elapsed_time >= sc.soft) break; 

        // 2) Estimate next time : Growth = num/den
        U64 factor_num = 1;
        U64 factor_den = 1;
        if(prev_time==0){
            factor_num = 2;
        }
        else{
            factor_num = depth_time; factor_den = prev_time;

            // make sure within bounds 
            if(factor_num * 2 < factor_den * 3){ // n/d < 1.5
                factor_num = 3; factor_den = 2;
            }
            else if(factor_num * 2 > factor_den * 8){ // n/d > 4.0
                factor_num = 8; factor_den = 2;
            }
        }

        // 3) Cut off based off next expected time
        U64 predicted_time = factor_num * depth_time / factor_den;
        U64 remaining_time = sc.soft-elapsed_time;

        if(predicted_time > remaining_time) break;

        prev_time = depth_time;
    } 
    
    if(DEBUG){ // debugging statements
        printf("\n    Nodes: %ld | Depth Reached %d | Time: %ld\n", sc.nodes, reached, get_time_ms() - sc.start);
        printf("    Evaluation %d\n", best.utility);
        printf("    ");
        print_move(best.move);
        printf("\n");
    }

    return best;
}

}