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

#include <iostream>
#include<iomanip>
using std::cout, std::endl;


// system headers
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_map>
#ifdef WIN64
    #include <windows.h>
#else
    # include <sys/time.h>
#endif

// bit board data type
#include <cstdint>
#include <cinttypes>

// define bitboard data type
// DEFINED IN BOARD

// FEN dedug positions
constexpr const char* empty_board =  "8/8/8/8/8/8/8/8 b - - ";
constexpr const char* start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
constexpr const char* tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
constexpr const char* killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
constexpr const char* cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
constexpr const char* kiwipete_position ="r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";

namespace bbc{


/**********************************\
 ==================================
 
           Random numbers
 
 ==================================
\**********************************/

// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number()
{
    // get current state
    unsigned int number = random_state;
    
    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    
    // update random number state
    random_state = number;
    
    // return random number
    return number;
}

// generate 64-bit pseudo legal numbers
U64 get_random_U64_number()
{
    // define 4 random numbers
    U64 n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    
    // return random number
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}



/**********************************\
 ==================================
 
          Bit manipulations
 
 ==================================
\**********************************/

// count bits within a bitboard (Brian Kernighan's way)
static inline int count_bits(U64 bitboard)
{
    // use built in functions    
    return __builtin_popcountll(bitboard);

}

// get least significant 1st bit index
static inline int get_ls1b_index(U64 bitboard)
{
     return (bitboard) ? __builtin_ctzll(bitboard) : -1;
}


/**********************************\
 ==================================
 
           Input & Output
 
 ==================================
\**********************************/

// print bitboard
void print_bitboard(U64 bitboard)
{
    // print offset
    printf("\n");

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // convert file & rank into square index
            int square = rank * 8 + file;
            
            // print ranks
            if (!file)
                printf("  %d ", 8 - rank);
            
            // print bit state (either 1 or 0)
            printf(" %d", get_bit(bitboard, square) ? 1 : 0);
            
        }
        
        // print new line every rank
        printf("\n");
    }
    
    // print board files
    printf("\n     a b c d e f g h\n\n");
    
    // print bitboard as unsigned decimal number
    printf("     Bitboard: %" PRIu64 "\n\n", bitboard);
}


// get time in milliseconds
int get_time_ms()
{
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
}

// leaf nodes (number of positions reached during the test of the move generator at a given depth)
long nodes;

// perft driver
static inline void perft_driver(Board& board, int depth)
{
    StateInfo st;

    // reccursion escape condition
    if (depth == 0)
    {
        // increment nodes count (count reached positions)
        nodes++;
        return;
    }
    
    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);

    // loop over generated moves
    int num_moves = move_list.count;
    
    for (int move_count = 0; move_count < num_moves; move_count++)
    {           
        // make move
        if (!make_move(move_list.moves[move_count], all_moves, board, st))
            // skip to the next move
            continue;

        // call perft driver recursively
        perft_driver(board, depth - 1);
        
        // take back
        restore_board(board, st, move_list.moves[move_count]);
    }
}

// perft test
void perft_test(Board& board, int depth)
{
    printf("\n     Performance test\n\n");
    
    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);

    StateInfo st;
    
    // init start time
    long start = get_time_ms();
    
    // loop over generated moves
    for (int move_count = 0; move_count < move_list.count; move_count++)
    {   
        // preserve board state
        Board copy;
        copy_board(copy, board);
        
        // make move
        if (!make_move(move_list.moves[move_count], all_moves, board, st))
            // skip to the next move
            continue;
        
        // cummulative nodes
        long cummulative_nodes = nodes;
        
        // call perft driver recursively
        perft_driver(board, depth - 1);
        
        // old nodes
        long old_nodes = nodes - cummulative_nodes;
        
        // take back
        restore_copy(copy, board);
        
        // print move
        printf("     move: %s%s%c  nodes: %ld\n", square_to_coordinates[get_move_source(move_list.moves[move_count])],
                                                 square_to_coordinates[get_move_target(move_list.moves[move_count])],
                                                 get_move_promoted(move_list.moves[move_count]) ? promoted_pieces[get_move_promoted(move_list.moves[move_count])] : ' ',
                                                 old_nodes);
    }
    
    // print results
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    printf("     Time: %ld\n\n", get_time_ms() - start);
}


/**********************************\
 ==================================
 
               Search

  AIMA CODE IMPLEMENTATION HERE
 
 ==================================
\**********************************/

// use a struct to store a move and its utility
struct move_utility {
    int utility;
    int move;
};

/***************************
    TRANSPOSITION TABLE 
/**************************/
// zobrist hashing functions for transposition
uint64_t random_pieces[768];
uint64_t random_side;
uint64_t random_castling[16];
uint64_t random_file[8];

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

uint64_t compute_zobrist_hash(Board& board){
    const auto& bitboards = board.bitboards;
    const auto& side = board.side;
    const auto& castle = board.castle;
    const auto& enpassant = board.enpassant;

    uint64_t hash = 0ULL;

    // calc hash with all piece bitboards
    for(int piece = P; piece <= k; piece++){
        uint64_t bitboard = bitboards[piece];

        // hash using xor of ALL piece positions
        while(bitboard){
            int square = __builtin_ctzll(bitboard);
            int addTable = piece * 64;

            hash ^= random_pieces[square + addTable];
            pop_bit(bitboard, square);
        }
    }

    // calc hash with side
    hash = side? hash : hash ^ random_side;

    // calc hash with castling
    hash ^= random_castling[castle];

    // calc hash with enpassant values
    if(enpassant <= 23) hash ^= random_file[enpassant - 16]; // squares 16 - 23 (a-h)
    else if (enpassant <= 47) hash ^= random_file[enpassant - 40]; // squares 40 - 47 (a-h)
 
    return hash;
}

// transposition 
// table entry
struct entry{
    uint64_t hash;
    int move;
    int depth;
    int utility;
    int node_type;
};

enum {LOWER_BOUND, EXACT, UPPER_BOUND};

const int TABLE_SIZE = 16 * 1024 * 1024; // hold around 64 million entries

// init table
entry transposition_table[TABLE_SIZE];

long invalid_table_moves = 0;
long table_moves = 0;
long stored = 0;
long probed = 0;

// get an entry
bool probe_entry(uint64_t hash, entry &entry, int depth){
    int index = hash % TABLE_SIZE;

    // only return if the table's stored hash is equivalent
    if(transposition_table[index].hash != 0) probed++;
    if(transposition_table[index].hash == hash){
        invalid_table_moves++;
        entry = transposition_table[index];
        return true;
    }

    return false;
}

// store entry inputs into table
void store_entry(uint64_t hash, int move, int depth, int utility, int node_type){
    int index = hash % TABLE_SIZE;
    stored++;
    // replace the hash if it has greater depth than current entry
    // ** MIGHT TRY AGING OR OTHER REPLACEMENT TECHNIQUES **
    if(transposition_table[index].depth <= depth || (node_type == EXACT && transposition_table[index].node_type != EXACT)){
        transposition_table[index] = {hash, move, depth, utility, node_type};
    }
    
}

/***************************
    QUIESCENCE SEARCH
/**************************/
int minQS(int alpha, int beta, Board& board);

// maximize quiescence search
int maxQS(int alpha, int beta, Board& board){
    int current_utility = eval(board);

    // beta cutoff
    if(current_utility >= beta) return beta;

    // update alpha if it's larger
    if(alpha < current_utility) alpha = current_utility;

    MoveList move_list;
    generate_moves(move_list, board);
    StateInfo st;

    for(int move = 0; move < move_list.count; move++){
        Board copy;
        copy_board(copy, board);

        if(!make_move(move, only_captures, board, st))
            continue;

        int utility = minQS(alpha,beta, board);

        restore_copy(copy, board);

        if( utility >= beta )
            return beta;
        if( utility > alpha )
           alpha = utility;
    }

    return alpha;
}

// minimize quiescence search
int minQS(int alpha, int beta, Board& board){
    int current_utility = eval(board);

    // alpha cutoff
    if(current_utility <= alpha) return alpha;

    // update beta if it's smaller
    if(beta > current_utility) beta = current_utility;

    MoveList move_list;
    generate_moves(move_list, board);
    StateInfo st;

    for(int move = 0; move < move_list.count; move++){
        Board copy;
        copy_board(copy, board);

        if(!make_move(move, only_captures, board, st))
            continue;

        int utility = maxQS(alpha,beta, board);

        restore_copy(copy, board);

        if( utility <= alpha )
            return alpha;
        if( utility < beta )
           beta = utility;
    }

    return beta;
}

/***************************
    CAPTURES V NON-CAPTURES 
/**************************/
// MVV LVA table
int MVV_LVA[12][12] = {
    // attacks by columns >
//   K   Q   R   B   N   P
    {60, 61, 62, 63, 64, 65, 60, 61, 62, 63, 64, 65}, // victims by columns V
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

/***************************
        KILLER MOVES
/**************************/
const int MAX_PLY = 256;
const int MAX_KILL_STORED = 2;
int killerMoves[MAX_PLY][MAX_KILL_STORED] = {};

// store killer moves to a stack
void storeKillerMove(int move, int ply){
    // exit capture moves
    if(get_move_capture(move)) return;

    // exit if move exists alr
    if(move == killerMoves[ply][0]) return;

    // shift moves FILO
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = move;
}


/***************************
        SORT MOVES  STILL NEEDS OPTIMIZATION !!! ! ! ! !!
/**************************/
void sortMoves(MoveList& move_list, bool probed, int TT_move, const Board& board){
    const auto& side = board.side;
    const auto& ply = board.ply;
    const auto& bitboards = board.bitboards;

    // Captures vs non captures
    int non_captures[256] = {};
    int nc_count = 0;
    int captures[256] = {};
    int c_count = 0;
    int added_moves = 0;

    int killer_moves[2] = {};
    int killer_index = 0;
    // check correct swaps
    // int initial[256] = {};

    // for(int i = 0; i < move_list->count; i++) initial[i] = move_list->moves[i];

    // Transposition move
    if(probed){
        int first_move = move_list.moves[0];
        // bool valid = false;
        for(int move_count = 0; move_count < move_list.count; move_count++){
            int move = move_list.moves[move_count];
            // TT move found
            if(move_count != 0 && move == TT_move){
                move_list.moves[0] = TT_move;
                move_list.moves[move_count] = first_move;
                added_moves += 1;
            }

            // Sort between captures
            else if(get_move_capture(move)){
                captures[c_count] = move;
                c_count++;
            }
            // Identify killer moves
            else if(move == killerMoves[ply][0]){
                killer_moves[0] = move;
            }
            else if(move == killerMoves[ply][1]){
                killer_moves[1] = move;
            }
            // Non capture moves
            else{
                non_captures[nc_count] = move;
                nc_count++;
            }
        }

         
        table_moves++;
    }
    // sort captures moves
    int* capture_scores = new int[c_count];
    for(int c= 0; c < c_count; c++){
        int capture_move = captures[c];
        // determine piece capture
        int source_piece = get_move_piece(capture_move);
        int target_square = get_move_target(capture_move);

        for(int target_piece = P; target_piece < k; target_piece++){

            // located correct piece
            if(get_bit(bitboards[target_piece], target_square)){
                // get score
                int mvv_score = MVV_LVA[11-target_piece][11-source_piece];
                capture_scores[c] = mvv_score;

                // sort the new score within the iterated pieces
                for(int sorted_location = 0; sorted_location < c; sorted_location++){
                    // located spot at sorted_move
                    if(mvv_score > capture_scores[sorted_location]){

                        // place the new score and shift down
                        for(int shift = c; shift > sorted_location; shift--){
                            capture_scores[shift] = capture_scores[shift-1];
                            // swap captures as well
                            captures[shift] = captures[shift-1];
                        }
                        // replace the new score
                        captures[sorted_location] = capture_move;
                        capture_scores[sorted_location] = mvv_score;
                        break;
                    }
                }
                break;
            }   
        }   
    }
    

    // add capture moves first
    for(int c = 0; c < c_count; c++){
        move_list.moves[added_moves] =  captures[c];
        // print_board();
        // print_move(move_list.moves[added_moves]);
        // printf("%d", capture_scores[c]);
        added_moves++;
    }
    delete[] capture_scores;

    // add killer moves FIFO
    if(killer_moves[0] != 0){
        move_list.moves[added_moves] = killer_moves[0];
        added_moves++;
    }
    if(killer_moves[1] != 0){
        move_list.moves[added_moves] = killer_moves[1];
        added_moves++;
    } 

    // add non captures moves
    for(int nc = 0; nc < nc_count; nc++){
        move_list.moves[added_moves] = non_captures[nc];
        added_moves++;
    }

    // check swaps
    // int final[256] = {};
    // for(int i = 0; i < move_list->count; i++) final[i] = move_list->moves[i];
    // ensure_same(initial, final, move_list->count);
}


/***************************
    ALPHA BETA SEARCH 
/**************************/
bool null = true;
long null_branches_pruned = 0;
long null_branches_explored = 0;

// forward min value to be used in mx
move_utility min_value(int alpha, int beta, int depth, Board& board);

// find max-value
// from AIMA, game is preserved in global array bitboards[] instead of an input, copy and takeback mimic this operation
move_utility max_value(int alpha, int beta, int depth, Board& board){
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;
    auto& ply = board.ply;

    // terminate at cutoff when depth is 0
    if(depth == 0) {
        // store move in transposition as exact
        int util = maxQS(alpha, beta, board);
        // store_entry(compute_zobrist_hash(), 0, depth, util, EXACT);
        nodes++;
        return {util, 0};
    }

    // probe transposition table for an entry
    entry ent;
    bool probed = probe_entry(compute_zobrist_hash(board), ent, depth);
    if(probed && ent.depth >= depth){
        // update alpha
        if(ent.node_type == LOWER_BOUND && ent.utility > alpha){
            alpha = ent.utility;
        }       

        // update beta
        else if(ent.node_type == UPPER_BOUND && ent.utility < beta){
            beta = ent.utility;
        }

        // return the node if its exact
        else if(ent.node_type == EXACT){
            nodes++;
            return {ent.utility, ent.move};
        }

        // cut off
        if(alpha >= beta){
            return {ent.utility, ent.move};
        }
    }

    // NULL MOVE
    if(null && alpha != beta -1){ // not a null branch
        if(depth >= 3 && ply >= 3){ // sufficient depth
            int white_king_square = __builtin_ctzll(bitboards[K]);
            int is_in_check = is_square_attacked(board, white_king_square, black);
            if(!is_in_check){ // not in check
                U64 pieces = bitboards[Q] | bitboards[R] | bitboards[B] | bitboards[N] |
                             bitboards[q] | bitboards[r] | bitboards[b] | bitboards[n];
                if(pieces){ // pieces present
                    if(eval(board) >= beta){ // position is good enough to continue
                        // null pruning
                        Board copy;
                        copy_board(copy, board);
                        side^=1;
                        ply++;
                        null_branches_explored++;
                        move_utility null_move = min_value(beta-1, beta, depth - 1 -2, board);
                        restore_copy(copy, board);

                        // fail high bc beta is still same
                        if(null_move.utility >= beta){
                            null_branches_pruned++;
                            return {beta, 0};
                        }
                        
                    }
                }
            }
        }
    }

    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);
    StateInfo st;


    // lost king is worst utility
    int current_utility = -20001;
    int current_move = 0;
    
    // ** NEEDS BETTER IMPLEMENTATION TO DETECT CHECKMATE VS STALEMATE *** 
    bool checkmate = true;

    // ** MOVE ORDERING ** //
    sortMoves(move_list, probed, ent.move, board);

    // loop over generated moves
    for (int move_count = 0; move_count < move_list.count; move_count++)
    {   
        // preserve board state
        Board copy;
        copy_board(copy, board);
        int move = move_list.moves[move_count];

        // make move
        if (!make_move(move, all_moves, board, st))
            // skip to the next move
            continue;
        
        checkmate = false;

        move_utility pair = min_value(alpha, beta, depth-1, board);
        
        // save the move pair if it has the greater utility than current best move
        if(pair.utility > current_utility){
            current_utility = pair.utility;
            current_move = move;

            // save the move to alpha if it has greater utility
            if(current_utility > alpha){
                alpha = current_utility; 
            } 
        }

        // take back
        restore_copy(copy, board);

        // beta cutoff
        if (current_utility >= beta) {
            // store killer move
            storeKillerMove(move, ply);

            // store move in transposition as lower bound
            store_entry(compute_zobrist_hash(board), move, depth, current_utility, LOWER_BOUND);
            nodes++;

            return {current_utility, current_move};
        }
    }

    // another terminal state
    if(checkmate){
        // no legal moves + in check = checkmate, otherwise stalemate
        int white_king_square = __builtin_ctzll(bitboards[K]);
        int is_in_check = is_square_attacked(board, white_king_square, white);
        nodes++;
        if(!is_in_check) return {0, 0};
        else return {-20000, 0};
    }
    // store move in transposition as exact
    store_entry(compute_zobrist_hash(board), current_move, depth, current_utility, EXACT);
    nodes++;
    return {current_utility, current_move};
}

move_utility min_value(int alpha, int beta, int depth, Board& board){
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;
    auto& ply = board.ply;

    // terminate at cutoff when depth is 0
    if(depth == 0) {
        // store move in transposition as exact
        int util = minQS(alpha, beta, board);

        // store_entry(compute_zobrist_hash(), 0, depth, util, EXACT);
        nodes++;
        return {util, 0};
    }
    
    // probe transposition table for an entry
    entry ent;
    bool probed = probe_entry(compute_zobrist_hash(board), ent, depth);
    if(probed && ent.depth >= depth){
        // update alpha
        if(ent.node_type == LOWER_BOUND && ent.utility > alpha){
            alpha = ent.utility;
        }       

        // update beta
        else if(ent.node_type == UPPER_BOUND && ent.utility < beta){
            beta = ent.utility;
        }

        // return the node if its exact
        else if(ent.node_type == EXACT){
            nodes++;
            return {ent.utility, ent.move};
        }

        // cutoff
        if(alpha >= beta){
            return {ent.utility, ent.move};
        }
    }

    // NULL MOVE
    if(null && alpha != beta -1){ // not a null branch
        if(depth >= 3 && ply >= 3){ // sufficient depth
            int black_king_square = __builtin_ctzll(bitboards[k]);
            int is_in_check = is_square_attacked(board, black_king_square, white);
            if(!is_in_check){ // not in check
                U64 pieces = bitboards[Q] | bitboards[R] | bitboards[B] | bitboards[N] |
                             bitboards[q] | bitboards[r] | bitboards[b] | bitboards[n];
                if(pieces){ // pieces present
                    if(eval(board) <= alpha){ // position is good enough to continue
                        // null pruning
                        Board copy;
                        copy_board(copy, board);
                        side^=1;
                        ply++;
                        null_branches_explored++;
                        move_utility null_move = max_value(alpha, alpha+1, depth - 1 -2, board);
                        restore_copy(copy, board);

                        // fail low bc alpha is still same
                        if(null_move.utility <= alpha){
                            null_branches_pruned++;
                            return {alpha, 0};
                        }
                    }
                }
            }
        }
    }
    

    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);
    StateInfo st;

    // ** MOVE ORDERING ** //
    sortMoves(move_list, probed, ent.move, board);

    // lost king is worst utility
    int current_utility = 20001;
    int current_move = 0;
    
    bool checkmate = true;

    // loop over generated moves
    for (int move_count = 0; move_count < move_list.count; move_count++)
    {   
        // preserve board state
        Board copy;
        copy_board(copy, board);
        
        int move = move_list.moves[move_count];
        // make move
        if (!make_move(move, all_moves, board, st))
            // skip to the next move
            continue;

        checkmate = false;

        move_utility pair = max_value(alpha, beta, depth-1, board);
        
        // save the move pair if it has the less utility than current best move
        if(pair.utility < current_utility){
            current_utility = pair.utility;
            current_move = move;

            // save to beta if it has less utility
            if(current_utility < beta){
                beta = current_utility; 
            } 
        }

        // take back
        restore_copy(copy, board);

        // alpha cutoff
        if (current_utility <= alpha) {
            // store killer move
            storeKillerMove(move, ply);

            // store move in transposition as upper bound
            store_entry(compute_zobrist_hash(board), current_move, depth, current_utility, UPPER_BOUND);
            nodes++;
            return {current_utility, current_move};
        }       
    }

    // another terminal state
    if(checkmate){
        // no legal moves + in check = checkmate, otherwise stalemate
        int black_king_square = __builtin_ctzll(bitboards[k]);
        int is_in_check = is_square_attacked(board, black_king_square, black);
        nodes++;
        if(!is_in_check) return {0, 0};
        else return {20000, 0};
    }
    // store move in transposition as exact
    store_entry(compute_zobrist_hash(board), current_move, depth, current_utility, EXACT);
    nodes++;
    return {current_utility, current_move};
}

// alpha beta search
move_utility alpha_beta_search(int depth, Board& board){
    long start = get_time_ms();
    nodes = 0;
    move_utility pair  = max_value(-20000, 20000, depth, board);
    printf("    Nodes: %ld\n", nodes);
    printf("    Time: %ld\n\n", get_time_ms() - start);
    printf("    Evaluation %d\n", pair.utility);
    print_move(pair.move);
    return pair;
}

// iterative deepening INCREMENTED AT .25 seconds
move_utility iterative_deepening(int depth, int time, Board& board){
    auto& side = board.side;
    long start = get_time_ms();
    nodes = 0;
    int reached = 0;
    move_utility pair;
    for(int i = 1; i <= depth; i++){
        // printf("time");
        if(get_time_ms() - start >= time * 250){
            break;
        }

        reached++;
        pair = side == white ? max_value(-20000, 20000, i, board) : min_value(-20000, 20000, i, board) ;
    }
    printf("\n    Nodes: %ld | Depth Reached %d | Time: %ld\n", nodes, reached, get_time_ms() - start);
    printf("    Evaluation %d\n", pair.utility);
    printf("    ");
    print_move(pair.move);
    printf("\n");
    return pair;
}

/**********************************\
 ==================================
 
                UCI
 
 ==================================
\**********************************/

//  user/GUI move string input (e.g. "e7e8q")
int parse_move(char *move_string, Board& board)
{
    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);
    
    // parse source square
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    
    // parse target square
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
    // loop over the moves within a move list
    for (int move_count = 0; move_count < move_list.count; move_count++)
    {
        // init move
        int move = move_list.moves[move_count];
        
        // make sure source & target squares are available within the generated move
        if (source_square == get_move_source(move) && target_square == get_move_target(move))
        {
            // init promoted piece
            int promoted_piece = get_move_promoted(move);
            
            // promoted piece is available
            if (promoted_piece)
            {
                // promoted to queen
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    // return legal move
                    return move;
                
                // promoted to rook
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    // return legal move
                    return move;
                
                // promoted to bishop
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    // return legal move
                    return move;
                
                // promoted to knight
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    // return legal move
                    return move;
                
                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }
            
            // return legal move
            return move;
        }
    }
    
    // return illegal move
    return 0;
}

/*
    Example UCI commands to init position on chess board
    play

    // init start position
    position startpos
    
    // init start position and make the moves on chess board
    position startpos moves e2e4 e7e5
    
    // init position from FEN string
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 
    
    // init position from fen string and make moves on chess board
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
*/

// parse UCI "position" command
void parse_position(char *command, Board& board)
{
    // shift pointer to the right where next token begins
    command += 9;
    
    // init pointer to the current character in the command string
    char *current_char = command;
    
    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
        // init chess board with start position
        board.parse_fen("r1bq1rk1/p1p2p2/2p2n1p/3p2p1/3QP3/2P3B1/P1P2PPP/R3KB1R w KQ - 2 12");
        //parse_fen(start_position);
    
    // parse UCI "fen" command 
    else
    {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");
        
        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            board.parse_fen("r1bqk2r/p1pp1p2/2p2n1p/6p1/4P2B/2P5/P1P2PPP/R2QKB1R w KQkq g6 0 10 ");
            
        // found "fen" substring
        else
        {
            // shift pointer to the right where next token begins
            current_char += 4;
            
            // init chess board with position from FEN string
            board.parse_fen(current_char);
        }
    }
    
    // parse moves after position
    current_char = strstr(command, "moves");
    
    // moves available
    if (current_char != NULL)
    {
        // shift pointer to the right where next token begins
        current_char += 6;
        
        // loop over moves within a move string
        while(*current_char)
        {
            // parse next move
            int move = parse_move(current_char, board);
            
            // if no more moves
            if (move == 0)
                // break out of the loop
                break;

            StateInfo st;
            
            // make move on the chess board
            make_move(move, all_moves, board, st);
            
            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;
            
            // go to the next move
            current_char++;
        }        
    }
    
    // print board
    board.print_board();
}

/*
    Example UCI commands to make engine search for the best move
    
    // fixed depth search
    go depth 64

*/

// parse UCI "go" command
void parse_go(char *command, Board& board)
{
    // init depth
    // int depth = 6;
    
    // // init character pointer to the current depth argument
    // char *current_depth = strstr(command, "depth");
    
    // // handle fixed depth search if "depth" is found in the command
    // if (current_depth) {
    //     // convert string to integer and assign the result to depth
    //     depth = atoi(current_depth + 6);  // Skip the "depth " part
    // }
    
    // search position with the given depth
    move_utility pair = iterative_deepening(25, 30, board);
    printf("Recommend move: ");
    print_move(pair.move);
    printf("Evaluation: %d\n", pair.utility);
}

/*
    GUI -> isready
    Engine -> readyok
    GUI -> uci
*/

// main UCI loop
void uci_loop(Board& board)
{
    auto& bitboards = board.bitboards;
    auto& side = board.side;
    auto& occupancies = board.occupancies;
    auto& enpassant = board.enpassant;
    auto& castle = board.castle;

    // reset STDIN & STDOUT buffers
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    // define user / GUI input buffer
    char input[2000];
    std::string line = "moves: ";
    
    // print engine info
    printf("commands: \n");
    printf("position startpos moves <start-move>\n");
    printf("play\n");
    printf("move <move>\n");
    printf("perft\n\n");
    
    // main loop
    while (1)
    {
        // reset user /GUI input
        memset(input, 0, sizeof(input));
        
        // make sure output reaches the GUI
        fflush(stdout);
        
        // get user / GUI input
        if (!fgets(input, 2000, stdin))
            // continue the loop
            continue;
        
        // make sure input is available
        if (input[0] == '\n')
            // continue the loop
            continue;
        
        // parse UCI "isready" command
        if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }
        
        // parse UCI "position" command
        else if (strncmp(input, "position", 8) == 0)
            // call parse position function
            parse_position(input, board);

        		// parse GUI input moves after initial position
		else if(!strncmp(input, "position startpos moves", 23))
		{
            printf("HELLO");
            board.print_board();  
        }
        else if(!strncmp(input, "play", 4))
		{   
            StateInfo st;
            if(side==white || side == black){
                 move_utility pair = iterative_deepening(9, 4, board);
                if(!make_move(pair.move, all_moves, board, st)) side ^= 1;    
                    else{
                        std::string  move_str = " " + move_string(pair.move);
                        line.append(move_str.c_str());  
                    } 
                board.print_board(); 
            }
        }

        else if(!strncmp(input, "move", 4))
		{   

            char *moves = input + 4; 
            
            while (*moves)
            {
                if (*moves == ' ')
                {
                    moves++;  
                    StateInfo st;
                    int move = parse_move(moves, board);  
                    make_move(move, all_moves, board, st); 
                    if(move!=0){
                        std::string  move_str = " " + move_string(move);
                        line.append(move_str.c_str());  
                    } 
                }
                moves++;  
            }

           
            board.print_board(); 
        }

        else if(!strncmp(input, "line", 4))
		{   
            printf("%s", line.c_str());
        }
        
        
        // parse UCI "perft" command
        else if (strncmp(input, "perft", 4) == 0)
        {
            int start = get_time_ms();

            nodes = 0;
            printf("Running perft... \n");
            // perft
            perft_driver(board, 6);
            // time taken to execute program
            printf("time taken to execute: %d ms\n", get_time_ms() - start);
            printf("nodes: %ld\n", nodes);
            break;
        }
        
        // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
            // call parse position function
            parse_position(const_cast<char*>("position startpos"), board);
        
        // parse UCI "go" command
        else if (strncmp(input, "go", 2) == 0)
            // call parse go function
            parse_go(input, board);
        
        // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0)
            // quit from the chess engine program execution
            break;
        
        // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0)
        {
            // print engine info
            printf("id name BBC\n");
            printf("id name \n");
            printf("uciok\n");
        }
    }
}


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

    // parse fen
    // #define empty_board "8/8/8/8/8/8/8/8 b - - "
    // #define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
    // #define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
    // #define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
    // #define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

    //parse_fen("r1bqk2r/p1p2p2/2p2n1p/3p2p1/4P3/2P3B1/P1P2PPP/R2QKB1R w KQkq d6 0 11");
    //null = true;
    //parse_fen("r1bq1rk1/p1p2p2/2p2n1p/3p2p1/3QP3/2P3B1/P1P2PPP/R3KB1R w KQ - 2 12");

    Board board;
    board.parse_fen(start_position);
    board.print_board();

    uci_loop(board);


    return 0;
}

}

int main(int argc, char* argv[]) {
    bbc::engine_main(argc, argv);
    return 0;
}
