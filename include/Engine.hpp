# pragma once
# include "Common.hpp"

namespace bbc{
    const int MAX_PLY = 512;
    const int MAX_KILL_STORED = 2;
    const bool DEBUG = true;
    struct SearchContext{
        U64 nodes = 0;
        
        int killerMoves[MAX_PLY][MAX_KILL_STORED] = {};

        bool null_enabled = true;

        // time management
        bool one_move = false;
        volatile bool stop = false;
        U64 hard = 0;
        U64 soft = 0;
        U64 start = 0;

        void clear();
    };

    

    // time management
    inline constexpr long OVERHEAD = 50; //30ms overhead

    struct TimeContext{
        U64 ms_left;
        U64 ms_inc;
        U64 start;
        U64 limit;

        void clear();
    };

    inline bool time_over_hard(const SearchContext& sc){
        return get_time_ms() - sc.start >= sc.hard;
    }

    // occasionaly check if we hit over the itme
    inline void poll_time(SearchContext& sc){
        // 0x3FFF = 16 000
        if (((++sc.nodes) & 1023) == 0 && time_over_hard(sc)) sc.stop = true;
    }
}