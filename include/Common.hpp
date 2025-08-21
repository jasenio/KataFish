// Common.hpp  (excerpt)
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#ifdef WIN64
    #include <windows.h>
#else
    # include <sys/time.h>
#endif
#include <chrono>

namespace bbc {

    using U64 = std::uint64_t;

    // encode pieces
    enum { no_piece = -1, P, N, B, R, Q, K, p, n, b, r, q, k };

    // bishop and rook
    enum { rook, bishop };

    // ASCII pieces (LEGACY DELTE ********* )
    inline constexpr std::string_view ascii_pieces = "PNBRQKpnbrqk";

    // unicode pieces
    inline constexpr const char* unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

    // fix later
    // inline constexpr std::string_view char_pieces = "PNBRQKpnbrqk";
    inline constexpr int char_pieces[256] = {
        /*   0- 63 */ -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /*  64- 79 */ -1,-1,  B,-1,-1,-1,-1,-1,  -1,-1,-1,  K,-1,-1,  N,-1,
        /*  80- 95 */  P,  Q,  R,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /*  96-111 */ -1,-1,  b,-1,-1,-1,-1,-1,  -1,-1,-1,  k,-1,-1,  n,-1,
        /* 112-127 */  p,  q,  r,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /* 128-159 */ -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /* 160-191 */ -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /* 192-223 */ -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,

        /* 224-255 */ -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1,
                    -1,-1,-1,-1,-1,-1,-1,-1,  -1,-1,-1,-1,-1,-1,-1,-1
    };
    inline constexpr int char_to_piece(char c) {
        return char_pieces[static_cast<unsigned char>(c)];
    }

    inline constexpr std::string_view promoted_pieces = "pnbrqkpnbrqk";

    // board squares
    enum {
        a8, b8, c8, d8, e8, f8, g8, h8,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a1, b1, c1, d1, e1, f1, g1, h1, no_sq
    };

    

    // sides to move (colors)
    enum { white, black, both };

    // castling rights binary encoding

    /*

        bin  dec
        
    0001    1  white king can castle to the king side
    0010    2  white king can castle to the queen side
    0100    4  black king can castle to the king side
    1000    8  black king can castle to the queen side

    examples

    1111       both sides an castle both directions
    1001       black king => queen side
                white king => king side

    */

    enum { wk = 1, wq = 2, bk = 4, bq = 8 };

    // convert squares to coordinates
    inline constexpr const char *square_to_coordinates[] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    };

    // ---------------------------------------------------------------
    // Lookup helpers that need those enums
    // ---------------------------------------------------------------
    // MAKE CONST LATER !! !! **** 

    // no_sq, set_bit, get_bit, … remain here too

    inline constexpr void set_bit(U64& bb, int sq) noexcept { bb |= 1ULL << sq; }
    inline constexpr bool get_bit(U64 bb, int sq) noexcept { return (bb >> sq) & 1ULL; }
    inline constexpr void pop_bit(U64& bb, int sq) noexcept { bb &= ~(1ULL << sq); }

    // get time in milliseconds
    inline U64 get_time_ms() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

    
    /**********************************\
     ==================================
    
            Random numbers
    
    ==================================
    \**********************************/

    // pseudo random number state
    inline unsigned int random_state = 1804289383;

    // generate 32-bit pseudo legal numbers
    inline unsigned int get_random_U32_number()
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
    inline U64 get_random_U64_number()
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

    // Search variables
    constexpr int MATE = 20000;
    constexpr int INF  = 30000;


} // namespace bbc
