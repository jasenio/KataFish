# pragma once
#include "Board.hpp"
#include "Movegen.hpp"

namespace bbc{

// perft driver
inline U64 perft_driver(Board& board, int depth)
{
    StateInfo st;

    U64 nodes = 0;

    // reccursion escape condition
    if (depth == 0)
    {
        // increment nodes count (count reached positions)
        nodes++;
        return nodes;
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
        nodes+=perft_driver(board, depth - 1);
        
        // take back
        restore_board(board, st, move_list.moves[move_count]);
    }
    return nodes;
}

// perft test
inline uint64_t perft_test(Board& board, int depth)
{
    printf("\n     Performance test\n\n");
    
    // create move list instance
    MoveList move_list;
    
    // generate moves
    generate_moves(move_list, board);

    StateInfo st;
    
    U64 nodes = 0;

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
        U64 cummulative_nodes = nodes;
        
        // call perft driver recursively
        perft_driver(board, depth - 1);
        
        // old nodes
        U64 old_nodes = nodes - cummulative_nodes;
        
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

    return nodes;
}

}