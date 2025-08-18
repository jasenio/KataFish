#pragma once
#include "Common.hpp"

namespace bbc{
    const int MAX_PLY = 256;
    const int MAX_KILL_STORED = 2;

    struct SearchContext{
        U64 nodes;
        
        int killerMoves[MAX_PLY][MAX_KILL_STORED] = {};

        bool null_enabled;
    };
}