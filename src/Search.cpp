#include "Search.hpp"

namespace bbc{

// q search at d=0
int qsearch(Board& board, int alpha, int beta){
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

    bool any = false;
    for (int i = 0; i < ml.count; ++i) {
        int move = ml.moves[i];
        
        if (!make_move(move, /*only_captures ok for speed*/ all_moves, board, st))
            continue;

        any = true;
        int score = -qsearch(board, -beta, -alpha);

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
    sc.nodes++; // count this node

    // 1: Quiescence Search at terminal nodes
    if (depth == 0) {
        int s = qsearch(board, alpha, beta);   // qsearch must be negamax-compatible
        return {s, 0};
    }

    int alpha0 = alpha;

    // 2: TT probe
    TTEntry ent;
    bool probed = tt.probe(get_hash(board), ent, depth);
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

    MoveList ml;
    generate_moves(ml, board);

    // 3: Sort better moves first
    sort_moves(ml, ent.move, board, tt, sc);

    // 4: Try making every legal move
    int bestScore = -INF;
    int bestMove  = 0;
    bool hasLegal = false;

    StateInfo st;
    for (int i = 0; i < ml.count; ++i) {
        int move = ml.moves[i];

        if (!make_move(move, all_moves, board, st)) continue;

        hasLegal = true;

        move_utility child = negamax(-beta, -alpha, depth - 1, board, tt, sc);
        int score = -child.utility;

        restore_board(board, st, move);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;

            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    storeKillerMove(move, board.ply, sc);
                    tt.store(get_hash(board), move, depth, bestScore, LOWER_BOUND);
                    return {bestScore, bestMove};
                }
            }
        }
    }

    // 5: Check for checkmate
    if (!hasLegal) {
        bool check = in_check_now(board);
        if (check)  return {-MATE, 0}; // or -MATE + ply for mate distance
        return {0, 0};                  // stalemate
    }

    // 6: Update node type and store in tt
    int t =
        (bestScore <= alpha0) ? UPPER_BOUND :
        (bestScore >= beta)   ? LOWER_BOUND :
                               EXACT;

    tt.store(get_hash(board), bestMove, depth, bestScore, t);
    return {bestScore, bestMove};
}

// iterative deepening INCREMENTED AT .25 seconds
move_utility iterative_deepening(int depth, int time, Board& board, TranspositionTable& tt, SearchContext& sc){
    long start = get_time_ms();
    sc.nodes = 0;
    int reached = 0;
    move_utility pair;
    for(int i = 1; i <= depth; i++){
        // printf("time");
        if(get_time_ms() - start >= time * 250){
            break;
        }

        reached++;
        pair = negamax(-INF, INF, i, board, tt, sc);
    }
    printf("\n    Nodes: %ld | Depth Reached %d | Time: %ld\n", sc.nodes, reached, get_time_ms() - start);
    printf("    Evaluation %d\n", pair.utility);
    printf("    ");
    print_move(pair.move);
    printf("\n");
    return pair;
}

}