# pragma once
# include "Board.hpp"
# include "Search.hpp"
# include "TT.hpp"
# include "MoveOrder.hpp"
# include "Engine.hpp"

#include<iostream>
#include<iomanip>
using std::cout, std::endl;

// system headers
#include <iostream>
#include <stdio.h>

namespace bbc{

// FEN dedug positions
inline constexpr const char* empty_board =  "8/8/8/8/8/8/8/8 b - - ";
inline constexpr const char* start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ";
inline constexpr const char* tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
inline constexpr const char* killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
inline constexpr const char* cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
inline constexpr const char* kiwipete_position ="r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";


//  user/GUI move string input (e.g. "e7e8q")
int parse_move(char *move_string, Board& board);
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
void parse_position(char *command, Board& board);
/*
    Example UCI commands to make engine search for the best move
    
    // fixed depth search
    go depth 64

*/

// parse UCI "go" command
void parse_go(char *command, Board& board, TimeContext& tc, TranspositionTable& tt, SearchContext& sc);

/*
    GUI -> isready
    Engine -> readyok
    GUI -> uci
*/
void uci_loop(Board& board, TimeContext& tc, TranspositionTable& tt, SearchContext& sc);


}