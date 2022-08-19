#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "tt.hpp"
#include "libchess/position.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

using namespace std;
using namespace libchess;


struct Node {
    std::int16_t lowerbound;
    std::int16_t upperbound;
};


//struct TTEntry {
//    bool root;
////    std::uint64_t hash;
//    std::unordered_map<std::tuple<bool, uint16_t, uint8_t, bool>, Node> u_map;
//};

struct TTEntry {
    bool root;
    std::uint64_t hash;
    std::int8_t depth;
    Node node;
};

struct TTEntry2 {
    std::uint64_t hash;
    Move m;
};


class Engine {

public:
    Engine(bool);

    int w_moves, b_moves;
    void setPosition(std::string, vector<std::string>);
    void setLimits(bool, int, bool, int, bool, int, bool);

    void think();
    int MTDF(bool, int16_t, uint8_t);
    int alphaBetaWithMemory(bool, int16_t, int16_t, int8_t, bool, bool, uint8_t);
    int qSearch(bool, int16_t, int16_t, int8_t, bool, bool, uint8_t);
    int evaluateBoard();

    bool haltFlag;
    bool waitTimeAfter;
    bool timePassedFlag;

    Side getTurn();

private:

    int nodes;
    uint8_t seldepth;

    const int16_t INFINITY = 10000;
    const int MAX_SEARCH_DEPTH = 100;

    const int ADJUST = 0;
    bool info;
    std::chrono::time_point<std::chrono::steady_clock> start_t;

    bool time_limit_present, depth_limit_present, node_limit_present;
    int t_limit, d_limit, n_limit;

    TT<TTEntry> tt{256};
    TT<TTEntry2> moves{64};

    libchess::Position board;
    libchess::Move bestMove;


};


#endif // ENGINE_H_INCLUDED
