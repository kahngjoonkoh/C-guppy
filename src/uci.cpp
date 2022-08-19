#include "argparse.hpp"
#include "FileLogger.hpp"
#include "libchess/position.hpp"
#include "engine.h"

#include <iostream>
#include <chrono>
#include <string>
#include <stack>
#include <algorithm>
#include <vector>
#include <thread>

using namespace std;


void halt(Engine &e, stack<std::string> &s) {
    std::string input;
    while (!e.haltFlag){
        getline(std::cin, input);
        if (input == "stop") {
            e.haltFlag = true;
            break;
        } else {
            s.push(input);
        }
    }
}




vector<std::string> splitString(std::string input, char separator) {

    size_t idx;
    vector<string> text;

    while (!input.empty()) {

        idx = input.find(separator); //find separator character position

        if (idx == string::npos) {
            text.push_back(input);
            break;
        }
        text.push_back(input.substr(0, idx));

        input = input.substr(idx + 1);
    }
    return text; //returns a vector with all the strings
}


int main(int argc, char *argv[]) {

    const char *version = "1.0";

//    argparse::ArgumentParser program("guppy", version);
//
//    program.add_argument("-h", "--hash")
//        .help("Sets the size(number of key-value pairs) of the hash table")
//        .default_value(256)
//        .nargs(1)
//        .scan<'i', int>();
//
////    program.add_argument("-n", "--nnue")
////        .help("the directory of the nnue file")
////        .nargs(argparse::nargs_pattern::optional)
////        .scan<'g', string>();
////
////    program.add_argument("-b", "--book")
////        .help("the directory of the openings")
////
////    program.add_argument("-t", "tablebase")
////        .help("the directory of endgame tablebases")
//
////
////    program.add_argument("-i", "--info")
////        .help("show engine info");
////
////    program.add_argument("-l", "--log")
////        .help("enables logging")
////        .default_value("log.txt")
////        .nargs(argparse::nargs_pattern::optional);
//
//    try {
//        program.parse_args(argc, argv);
//    }
//
//    catch (const std::runtime_error &err) {
//        std::cerr << err.what() << std::endl;
//        std::cerr << program;
//        std::exit(1);
//    }

//    FileLogger logger(version, program.is_used("--log"), program.get<std::string>("--log").c_str());
    FileLogger logger(version, false,"log.txt");

    std::string cmd;
    stack<std::string> cmdStack;
    std::string fen;
    bool show_thinking = true;
    Engine guppy(show_thinking);

    while (true) {
        if (cmdStack.empty()) {
            try {
//                std::cin >> cmd;
                getline(std::cin, cmd);
            }
            catch (int e) {
                std::cout << e;
                logger << std::to_string(e);
            }
        } else {
            cmd = cmdStack.top();
            cmdStack.pop();
        }

        logger << cmd;

        if (cmd == "quit") {
            guppy.haltFlag = true;
            std::exit(1);

        } else if (cmd == "uci") {
            std::cout << "id name Guppy"<< std::endl;
            std::cout << "id author Kahngjoon Koh"<< std::endl;
            std::cout << "uciok"<< std::endl;

        } else if (cmd == "debug") {
            ;
        } else if (cmd == "isready") {
            std::cout << "readyok" << std::endl;

        } else if (cmd == "setoption") {
            ;
        } else if (cmd == "register") {
            ;
        } else if (cmd == "ucinewgame") {
            cmdStack.push("position startpos");

        } else if (cmd.starts_with("position")) {
            vector<std::string> params = splitString(cmd.substr(9), ' ');
            vector<std::string> moves;
            std::string fen;
            int idx = 0;

            auto it = std::find(params.begin(), params.end(), "moves");
            if ( it != params.end())  {
                idx = it - params.begin();
                moves = std::vector<std::string> (params.begin() + idx + 1, params.end());
            }

            if (params.front() == "startpos") {
                fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

            } else if (params.front() == "fen") {
                size_t found = cmd.find("moves");
                if (found != std::string::npos)  {
                    fen = cmd.substr(13, found - 14);
                } else {
                    fen = cmd.substr(13);
                }
            } else {
                logger << FileLogger::LOG_ERROR << "Invalid Command.";
            }

            guppy.setPosition(fen, moves);

            //TODO
        } else if (cmd.starts_with("go")) {
            vector<std::string> go_params = splitString(cmd.substr(3), ' ');
            std::string info_str;

            if (go_params.front() == "infinite") {
                std::thread th1(halt, std::ref(guppy), std::ref(cmdStack));
                guppy.setLimits(false, 0, false, 0, false, 0, false);
                guppy.think();
                th1.join();

                // no support for searchmoves and ponder
            } else {
                bool limit_time=false, limit_depth=false, limit_nodes=false, movestogo_given=false, movetime_given=false;
                int wtime=0, btime=0, winc=0, binc=0, movestogo=0, movetime=0, depth=0, nodes=0, time_control=0;
                bool timeAfter = true;
                vector<std::string>::iterator it;

                int i = 0;
                for(it = go_params.begin(); it != go_params.end(); it++,i++ ) {
                // found nth element..print and break.
                    if (*it == "wtime") {
                        limit_time = true;
                        wtime = std::stoi(go_params[i+1]);
                    } else if (*it == "btime") {
                        limit_time = true;
                        btime = std::stoi(go_params[i+1]);
                    } else if (*it == "winc") {
                        limit_time = true;
                        winc = std::stoi(go_params[i+1]);
                    } else if (*it == "binc") {
                        limit_time = true;
                        binc = std::stoi(go_params[i+1]);
                    } else if (*it == "movestogo") {
                        movestogo_given = true;
                        movestogo = std::stoi(go_params[i+1]);
                    } else if (*it == "depth") {
                        limit_depth = true;
                        depth = std::stoi(go_params[i+1]);
                    } else if (*it == "nodes") {
                        limit_nodes = true;
                        nodes = std::stoi(go_params[i+1]);
                    } else if (*it == "movetime") {
                        movetime_given = true;
                        movetime = std::stoi(go_params[i+1]);
                    }
                }
                if (timeAfter) {
                    time_control = (guppy.getTurn() == libchess::Side::White) ? (wtime/50 + winc/2) : (btime/50 + binc/2);
                    guppy.setLimits(limit_time, time_control, limit_depth, depth, limit_nodes, nodes, true);
                }
                 else if (movestogo_given) {
                    time_control = (guppy.getTurn() == libchess::Side::White) ? (wtime/(movestogo + 1) + winc) : (btime/(movestogo + 1) + binc);
                    guppy.setLimits(limit_time, time_control, limit_depth, depth, limit_nodes, nodes, false);

                } else if (movetime_given) {
                    guppy.setLimits(true, movetime, limit_depth, depth, limit_nodes, nodes, false);
                } else {
                    // 40 for human, 60 for engines.
                    int av_ply = 50;
                    time_control = (guppy.getTurn() == libchess::Side::White) ? (wtime/(av_ply-guppy.w_moves + 1) + winc) : (btime/(av_ply-guppy.b_moves + 1) + binc);
                    guppy.setLimits(limit_time, time_control, limit_depth, depth, limit_nodes, nodes, false);
                }
                std::thread th2(halt, std::ref(guppy), std::ref(cmdStack));
                guppy.think();
                th2.join();
                std::cout << "thread joined"<<std::endl;
            }
        } else if (cmd == "ponderhit") {
            ;
        } else {
            logger << FileLogger::LOG_ERROR << "Unknown Command";
        }

    }

    return 0;
}

