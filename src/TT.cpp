# include "TT.hpp"

namespace bbc{

// constructor
TranspositionTable::TranspositionTable(size_t mb){
    size_t bytes = mb * 1024 * 1024;
    size_t num_entries = bytes / sizeof(TTEntry);
    num_entries = (1 << std::__lg(num_entries));

    table = new TTEntry[num_entries]();
    size = num_entries;

    // debugging
    probed = 0;
    invalid_table_moves = 0;
    stored = 0;
    used = 0;
}

// destructor
TranspositionTable::~TranspositionTable(){
    delete[] table;
}

// resize
void TranspositionTable::resize(size_t mb){
    size_t bytes = mb * 1024 * 1024;
    size_t num_entries = bytes / sizeof(TTEntry);
    num_entries = (1 << std::__lg(num_entries)); // use msb for num_entries

    TTEntry* newTable = new TTEntry[num_entries]();
    
    for(int i = 0; i < num_entries; i++){
        newTable[i] = table[i];
    }

    delete[] table;
    table = newTable;

    size = num_entries;
}

// clear all entries
void TranspositionTable::clear(){
    for(size_t i = 0; i < size; i++){
        table[i] = TTEntry();
    }
}

// get an entry
bool TranspositionTable::probe(U64 hash, TTEntry &out, int depth) {
    int index = hash % size;

    // only return if the table's stored hash is equivalent
    if(table[index].key != 0) probed++;
    if(table[index].key == hash){
        invalid_table_moves++;
        out = table[index];
        return true;
    }

    return false;
}

// store entry inputs into table
void TranspositionTable::store(U64 hash, int move, int depth, int utility, int node_type){
    int index = hash % size;
    stored++;
    // replace the hash if it has greater depth than current entry
    // ** MIGHT TRY AGING OR OTHER REPLACEMENT TECHNIQUES **
    if(table[index].depth <= depth || (node_type == EXACT && table[index].node_type != EXACT)){
        table[index] = {hash, move, depth, utility, node_type};
    }
    
}

// increased used moves from table
void TranspositionTable::inc_used(){ used++; }



}