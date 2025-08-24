# include "Engine.hpp"

namespace bbc{

void SearchContext::clear(){
    this->nodes = 0;
    
    for(int ply = 0; ply < MAX_PLY; ply++){
        for(int move = 0; move < MAX_KILL_STORED; move++){
            killerMoves[ply][move] = 0;
        }
    }

    this->null_enabled = true;

    this->one_move = false;
    this->stop = false;
    this->hard = 0;
    this->soft = 0;
    this->start = 0;
}

void TimeContext::clear(){
    this->ms_inc = 0;
    this->ms_left = 0;
    this->start = 0;
    this->limit = 0;
}

}