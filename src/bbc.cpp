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

#include<iostream>
#include<iomanip>
using std::cout, std::endl;

// system headers
#include <iostream>
#include <stdio.h>

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
void parse_go(char *command, Board& board, TranspositionTable& tt, SearchContext& sc)
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
    move_utility pair = iterative_deepening(25, 30, board, tt, sc);
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
void uci_loop(Board& board, TranspositionTable& tt, SearchContext& sc)
{
    auto& side = board.side;

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
                 move_utility pair = iterative_deepening(9, 4, board, tt, sc);
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

            U64 nodes = 0;
            printf("Running perft... \n");
            // perft
            nodes = perft_driver(board, 6);
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
            parse_go(input, board, tt, sc);
        
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
    constexpr const char * puzz = "8/8/8/4b2k/8/7p/8/7K w - - 1 52";
    Board board;
    board.parse_fen(start_position);
    board.print_board();
    TranspositionTable tt;
    SearchContext sc;

    uci_loop(board, tt, sc);


    return 0;
}

}

int main(int argc, char* argv[]) {
    bbc::engine_main(argc, argv);
    return 0;
}
