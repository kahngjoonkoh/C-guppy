#include "engine.h"

#include "libchess/position.hpp"
#include "tt.hpp"

#include <algorithm>
#include <iostream>
#include <chrono>


Engine::Engine(bool showThinking) {
    nodes = 0;
    board.init_tables();
    info = showThinking;

}

void Engine::setPosition(std::string fen, vector<std::string> moves) {
    auto pos = libchess::Position(fen);
    w_moves = 0;
    b_moves = 0;
    for (std::string move_str: moves) {
        if (pos.turn() == libchess::Side::White) {
            w_moves++;
        } else
            b_moves++;
        pos.makemove(move_str);
    }
    board = pos;
}

void Engine::setLimits(bool tb, int t, bool db, int d, bool nb, int n, bool timeAfter) {
    time_limit_present = tb;
    depth_limit_present = db;
    node_limit_present = nb;

    t_limit = t;
    d_limit = d;
    n_limit = n;
    waitTimeAfter = timeAfter;
    timePassedFlag = false;
}

void Engine::think() {
    nodes = 0;

    int16_t fg = 0;
    haltFlag = false;

    vector<Move> legals = board.legal_moves();
    bestMove = legals.front(); // fallback if no depth given.
    start_t = chrono::steady_clock::now();
    for (int depth = 1; depth < MAX_SEARCH_DEPTH; depth++ ) {
        seldepth = 0;

        // mate detected.
        if (abs(fg) == INFINITY) {
            haltFlag = true;
            break;
        }

        if (waitTimeAfter && timePassedFlag) {
            haltFlag = true;
            break;
        }


        if (depth_limit_present && (depth > d_limit)) {
            haltFlag = true;
            break;
        }

        fg = MTDF(true, fg, depth);

        if (!haltFlag)
            bestMove = moves.poll(board.hash()).m; //best move fallback
        else
            break;
        // score to engine perspective.
        int score = (board.turn() == libchess::Side::White) ? fg : -fg;
//        int score = fg;
        auto end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start_t).count();
        int nps = (elapsed == 0) ? 0 : (float) nodes/ (float) elapsed * 1000;
        if (info) {
            std::cout<<"info depth "<<depth<<" seldepth "<<depth+seldepth<<" score cp "<<score<<" time "<<elapsed<<" nodes "<<nodes<<" nps "<<nps<<" pv "<<bestMove<< std::endl;
        }
    }
    // currently doesn't support ponder.
    std::cout << "bestmove " << bestMove << std::endl;
}

int Engine::MTDF(bool root, int16_t fg, uint8_t depth) {
    int16_t g = fg;
    int16_t upperbound = INFINITY;
    int16_t lowerbound = -INFINITY;

    while (lowerbound < upperbound - ADJUST) {
        int16_t beta = max(g, static_cast<int16_t>(lowerbound + 1));
        bool maximise = (board.turn() == libchess::Side::White);
        g = alphaBetaWithMemory(root, beta - 1, beta, depth, false, maximise, 0);
        if (!haltFlag) {
            if (g < beta)
                upperbound = g;
            else
                lowerbound = g;
        } else
            break;
    }
    return g;
}

int Engine::alphaBetaWithMemory(bool root, int16_t alpha, int16_t beta, int8_t depth, bool quiet, bool maximise, uint8_t i) {


    // checkup every 1024 nodes
    if ((nodes & 1023) == 0) {
        if (time_limit_present) {
            if (!timePassedFlag) {
                auto end = chrono::steady_clock::now();
                auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start_t).count();
                if (elapsed > t_limit) {
                    if (waitTimeAfter) {
                        timePassedFlag = true;
                        haltFlag = true;
                        return 0;
                    } else {
                        timePassedFlag = true;
                    }
                }
            }

        }
    }

    if (node_limit_present && (nodes >= n_limit)) {
        haltFlag = true;
        return 0;
    }

    if (haltFlag) {
        return 0;
    }

    nodes++;

    if (board.is_checkmate()) {
        return (board.turn() == libchess::Side::White) ? -INFINITY : INFINITY;
    }

    if (board.is_stalemate()) {
        return 0;
    }
    depth = max(depth, static_cast<int8_t>(0));

    Node n;
    const auto &entry = tt.poll(board.hash());
        if (entry.hash == board.hash() && entry.depth==depth && entry.root == root) {
            n = entry.node;

            if (n.lowerbound >= beta)
                return n.lowerbound;
            if (n.upperbound <= alpha)
                return n.upperbound;
            alpha = max(static_cast<int16_t>(alpha), n.lowerbound);
            beta = min(static_cast<int16_t>(beta), n.upperbound);
        } else
            n = {static_cast<int16_t>(-INFINITY), static_cast<int16_t>(INFINITY)};

    if (depth == 0) {
        if (quiet) {
            seldepth = max(seldepth, i);
            return evaluateBoard();
        } else
            i++;
//            return qSearch(bool root, int16_t alpha, int16_t beta, int8_t depth, bool quiet, bool maximise, uint8_t i);
    }
//
//    vector<Move> legals;
//    if (depth == 0) {
//        auto A = board.legal_captures();
//        auto B = board.check_evasions();
//        legals.reserve( A.size() + B.size() ); // preallocate memory
//        legals.insert( legals.end(), A.begin(), A.end() );
//        legals.insert( legals.end(), B.begin(), B.end() );
//    } else {
//        legals = board.legal_moves();
//    }

    auto legals = board.legal_moves();


    int g, a, b;
    if (maximise) {
        g = -INFINITY;
        a = alpha;

        for (Move m: legals) {
            board.makemove(m);

            g = max(g, alphaBetaWithMemory(false, a, beta, depth - 1, (board.in_check() || m.is_capturing()) ? 0 : 1, !maximise, i));
            a = max(a, g);
            board.undomove();

            if (g >= beta) {
                if (!haltFlag) {
                   moves.add(board.hash(), {board.hash(), m});
                }
                break;
            }
        }

    } else { // if minimise
        g = INFINITY;
        b = beta;

        for (Move m: legals) {

            board.makemove(m);
            g = min(g, alphaBetaWithMemory(false, alpha, b, depth - 1, (board.in_check() || m.is_capturing()) ? 0 : 1, !maximise, i));
            b = min(b, g);
            board.undomove();
            if (g <= alpha){
                if (!haltFlag){
                    moves.add(board.hash(), {board.hash(), m});
                }
                break;
            }
        }

    }

    if (!haltFlag) {
        // when fail-low
        if (g <= alpha)
            n.upperbound = g;
        else if (g > alpha && g < beta) {
            n.lowerbound = g;
            n.upperbound = g;
        // when fail-high
        } else if (g >= beta)
            n.lowerbound = g;

        tt.add(board.hash(), {root, board.hash(), depth, n});
    }
    return g;

}

int Engine::qSearch(bool root, int16_t alpha, int16_t beta, int8_t depth, bool quiet, bool maximise, uint8_t i) {
    return 0;
}

int Engine::evaluateBoard() {
//    return board.turn() ? -board.pesto_eval() : board.pesto_eval();
    return board.pesto_eval();
}

Side Engine::getTurn() { return board.turn();}
