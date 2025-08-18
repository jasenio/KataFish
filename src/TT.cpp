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

// random pieces
U64 random_pieces[768];
U64 random_side;
U64 random_castling[16];
U64 random_file[8];

// init zobrist table
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

// get hash
U64 get_hash(Board& board){
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

}