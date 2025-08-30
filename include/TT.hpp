#pragma once
#include <cstdint>
#include <stddef.h>

#include "Common.hpp"
#include "Board.hpp"

namespace bbc{

// define table entries
enum {LOWER_BOUND, EXACT, UPPER_BOUND};

struct TTEntry {
    U64 key = 0;
    int move = 0;
    int depth = 0;
    int value = 0;
    int node_type = 0; // EXACT / LOWER / UPPER
};

// define object class
class TranspositionTable {
public:
    TranspositionTable(size_t mb = 64);

    void resize(size_t mb);

    void clear();

    bool probe(U64 key, TTEntry& out, int depth);
    void store(U64 key, int move, int depth, int utility, int node_type);
    void inc_used();
    inline size_t getSize() const {return size;}
    ~TranspositionTable();

private:
    TTEntry* table;
    size_t size;
    
    size_t probed;
    size_t invalid_table_moves;
    size_t stored;
    size_t used;
};




}