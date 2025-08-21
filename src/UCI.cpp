# include "UCI.hpp"
# include "Perft.hpp"

namespace bbc{
// helpers
static inline int read_after(const char* s, const char* key, int fallback) {
    const char* p = strstr(s, key);
    return p ? std::max(0, atoi(p + std::strlen(key))) : fallback;
}

static inline bool starts_with(const char* s, const char* pfx) {
    return std::strncmp(s, pfx, std::strlen(pfx)) == 0;
}

//  user/GUI move string input (e.g. "e7e8q")
int parse_move(char* move_string, Board& board)
{
    MoveList move_list;
    generate_moves(move_list, board);

    // e2e4 → (file='e'→4) + (rank='2'→6*8); but we map as in user's existing code:
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;

    for (int i = 0; i < move_list.count; i++) {
        int move = move_list.moves[i];
        if (get_move_source(move) == source_square &&
            get_move_target(move) == target_square)
        {
            int promoted_piece = get_move_promoted(move);
            if (promoted_piece) {
                // guard for short strings
                char promo = move_string[4] ? move_string[4] : '\0';

                if ((promoted_piece == Q || promoted_piece == q) && promo == 'q') return move;
                if ((promoted_piece == R || promoted_piece == r) && promo == 'r') return move;
                if ((promoted_piece == B || promoted_piece == b) && promo == 'b') return move;
                if ((promoted_piece == N || promoted_piece == n) && promo == 'n') return move;

                // wrong promotion letter → continue looking
                continue;
            }
            return move;
        }
    }
    return 0; // illegal / not found
}

// parse UCI "position" command
void parse_position(char* command, Board& board)
{
    // advance past "position "
    command += 9;

    static const char* START_FEN =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Default: startpos
    if (std::strncmp(command, "startpos", 8) == 0) {
        board.parse_fen(const_cast<char*>(START_FEN));
        command = std::strstr(command, "moves"); // may be null
    } else {
        // expect "fen <FEN...>"
        char* fen_ptr = std::strstr(command, (char*)"fen");
        if (!fen_ptr) {
            board.parse_fen(const_cast<char*>(START_FEN));
            command = std::strstr(command, (char*)"moves"); // may be null
        } else {
            fen_ptr += 4; // skip "fen "

            // copy FEN up to " moves" or end
            char fen_buf[256] = {0};
            size_t i = 0;
            while (fen_ptr[i] && i < sizeof(fen_buf) - 1) {
                if (!std::strncmp(&fen_ptr[i], " moves", 6)) break;
                fen_buf[i] = fen_ptr[i];
                ++i;
            }
            fen_buf[i] = '\0';
            board.parse_fen(fen_buf);

            command = std::strstr(fen_ptr, "moves"); // may be null
        }
    }

    // apply moves if present
    if (command) {
        command += 6; // skip "moves "
        while (*command) {
            while (*command == ' ') ++command;
            if (!*command) break;

            int mv = parse_move(command, board);
            if (!mv) break;

            StateInfo st;
            make_move(mv, all_moves, board, st);

            // advance to next token
            while (*command && *command != ' ') ++command;
        }
    }
}
/*
    Example UCI commands to make engine search for the best move
    
    // fixed depth search
    go depth 64

*/

// parse UCI "go" command
void parse_go(char* command, Board& board, TimeContext& tc,
              TranspositionTable& tt, SearchContext& sc){
    // inside parse_go(...)
    tc.clear();

    int depth    = -1;
    int movetime = -1;
    int wtime = -1, btime = -1, winc = 0, binc = 0;

    // parse tokens (use your own helpers if you have them)
    if (const char* p = strstr(command, "depth"))     depth    = std::max(1, atoi(p + 5));
    if (const char* p = strstr(command, "movetime"))  movetime = std::max(1, atoi(p + 8));
    if (const char* p = strstr(command, "wtime"))     wtime    = std::max(0, atoi(p + 5));
    if (const char* p = strstr(command, "btime"))     btime    = std::max(0, atoi(p + 5));
    if (const char* p = strstr(command, "winc"))      winc     = std::max(0, atoi(p + 4));
    if (const char* p = strstr(command, "binc"))      binc     = std::max(0, atoi(p + 4));

    // Map UCI clocks → your TimeContext
    if (movetime > 0) {
        // Fixed per-move time
        tc.ms_left = movetime;
        tc.ms_inc  = 0;
        sc.one_move = true;
    } else if (wtime >= 0 && btime >= 0) {
        // Use our own clock and increment depending on side to move
        if (board.side == white) { tc.ms_left = wtime; tc.ms_inc = winc; }
        else                      { tc.ms_left = btime; tc.ms_inc = binc; }
    } else {
        // Depth-only (no time control given)
        tc.ms_left = 0;
        tc.ms_inc  = 0;
    }

    tc.start = get_time_ms();

    // Now your existing policy can stay as-is:
    sc.nodes = 0;
    sc.start = tc.start;
    sc.stop = false;

    // --- budgets ---
    if (movetime > 0) { // one move
        U64 soft = std::max<U64>(20, tc.ms_left - OVERHEAD - 5);
        U64 hard = std::min<U64>(tc.ms_left - OVERHEAD,
                                soft + std::max<long>(30, soft/10));   // ✅ clamp
        sc.soft = soft;
        sc.hard = hard;
    } else if (tc.ms_left > 0) { // time + increment
        U64 base   = tc.ms_left / 40;               // ~2.5%
        U64 inc    = (tc.ms_inc) / 2;          // 50% increment
        U64 budget = std::max<U64>(20, std::min<long>(base + inc, tc.ms_left / 2));
        U64 hardCap= std::max<U64>(budget, (4 * tc.ms_left) / 5 - OVERHEAD);
        sc.soft = budget;
        sc.hard = std::max<U64>(20, hardCap);       // ✅ never negative
    } else { //infinite move
        sc.soft = __LONG_MAX__ / 4;                      // ✅ standard macro
        sc.hard = __LONG_MAX__ / 4;
    }

    // depth precedence stays the same
    int searchDepth = (depth > 0 ? depth : 99);
    if(DEBUG) std::cout << sc.soft << " " << sc.hard << " " << sc.start<<endl; // write time allocations

    move_utility best = iterative_deepening(searchDepth, tc, board, tt, sc);

    // Always output something valid
    const std::string bm = move_string(best.move);
    printf("bestmove %s\n", bm.empty() ? "0000" : bm.c_str());

}

/*
    GUI -> isready
    Engine -> readyok
    GUI -> uci
*/

// main UCI loop
void uci_loop(Board& board, TimeContext& tc,  TranspositionTable& tt, SearchContext& sc)
{

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
        std::memset(input, 0, sizeof(input));
        
        // make sure output reaches the GUI
        std::fflush(stdout);

        if (!std::fgets(input, sizeof(input), stdin)) continue;
        if (input[0] == '\n') continue;
        
        // parse UCI "isready" command
        if (starts_with(input, "uci")) {
            std::printf("id name JJK\n");
            std::printf("id author jasenio\n");
            // Print options here if/when you support them, e.g.:
            // std::printf("option name Hash type spin default 16 min 1 max 4096\n");
            std::printf("uciok\n");
        }
        else if (starts_with(input, "isready")) {
            std::printf("readyok\n");
        }
        else if (starts_with(input, "ucinewgame")) {
            parse_position(const_cast<char*>("position startpos"), board);
            tc.clear();
            tt.clear();
            sc.clear();         // ensure this resets any stop flag in your search
        }
        else if (starts_with(input, "position")) {
            parse_position(input, board);
        }
        else if (starts_with(input, "go")) {
            parse_go(input, board, tc, tt, sc);
        }
        else if (starts_with(input, "stop")) {
            // Your search should poll `sc.stop` or the time budget and exit ASAP.
            sc.stop = true;     // add a bool/atomic in SearchContext if not present
        }
        else if (starts_with(input, "quit")) {
            break;
        }
        else if (starts_with(input, "setoption")) {
            // Optional: ignore unknown options gracefully for now.
            // You can later parse "name Hash value N" and call tt.resize_mb(N).
        }
        else if (starts_with(input, "perft")) {
            // Dev-only: keep noise off stdout so GUIs aren't confused
            int start = get_time_ms();
            U64 nodes = perft_driver(board, 6);
            int ms = get_time_ms() - start;
            std::fprintf(stderr, "perft nodes=%llu time=%dms\n",
                         (unsigned long long)nodes, ms);
        }
        // else: ignore unknown commands quietly
    }
}

}