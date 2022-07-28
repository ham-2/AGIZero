#include <iostream>
#include <deque>
#include <memory>

#include "constants.h"
#include "position.h"
#include "misc.h"
#include "search.h"
#include "uci.h"
#include "eval.h"
#include "table.h"
#include "options.h"


std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

int main()
{
    using namespace std;
    using namespace AGI;
    using namespace Stockfish;

    //initialize
    Bitboards::init();
    Position::init();
    psqt_init();
    Main_TT.clear();
    Pawn_TT.clear();
    Threads.init();

    string input;
    deque<string> input_split;
    std::mutex ready_mutex;
    
    Position board;
    StateInfo* si = new Stockfish::StateInfo;
    board.set(DEFAULT_FEN, false, si);
    Threads.add_one(DEFAULT_FEN); // [0] is Printboard

    while (true) {
        getline(cin, input);

        while (SEARCH::ponder_continue) { this_thread::sleep_for(chrono::milliseconds(2)); }
        Threads.stop = true;

        ready_mutex.lock();
        ready_mutex.unlock();

        input_split = split(input);
        if (input_split.front() == "quit") { break; }

        else if (input_split.front() == "uci") {
            std::cout << "id name AGI\n"
                << "id author Seungrae Kim" << endl;
            // Options
            print_option();
            cout << "uciok" << endl;
        }

        else if (input_split.front() == "isready") {
            std::cout << "readyok" << endl;
        }

        else if (input_split.front() == "ucinewgame") {
            Main_TT.clear();
            Pawn_TT.clear();
            Stockfish::StateInfo* si = new Stockfish::StateInfo;
            board.set(DEFAULT_FEN, false, si);
            Threads.set_fen(DEFAULT_FEN);
        }

        else if (input_split.front() == "position") {
            input_split.pop_front();

            if (input_split.front() == "startpos") {
                input_split.pop_front();

                if (!input_split.empty()) {
                    StateInfo* si = new Stockfish::StateInfo;
                    board.set(DEFAULT_FEN, false, si);
                    Threads.set_fen(DEFAULT_FEN);

                    if (input_split.front() == "moves") {
                        input_split.pop_front();
                        while (!input_split.empty()) {
                            Stockfish::Move move = AGI::to_move(board, input_split.front());
                            board.do_move(move);
                            Threads.do_move(move);
                            input_split.pop_front();
                        }
                    }
                }
                else {
                    Stockfish::StateInfo* si = new Stockfish::StateInfo;
                    board.set(DEFAULT_FEN, false, si);
                    Threads.set_fen(DEFAULT_FEN);
                }
            }
            else if (input_split.front() == "fen") {
                input_split.pop_front();
                string fen = getfen(input_split);
 
                Stockfish::StateInfo* si = new Stockfish::StateInfo;
                board.set(fen, false, si);
                Threads.set_fen(fen);

                for (int i = 0; i < 6; i++) { input_split.pop_front(); }
                if (!input_split.empty()) {
                    if (input_split.front() == "moves") {
                        input_split.pop_front();
                        while (!input_split.empty()) {
                            Stockfish::StateInfo* si = new Stockfish::StateInfo;
                            Stockfish::Move move = AGI::to_move(board, input_split.front());
                            board.do_move(move);
                            Threads.do_move(move);
                            input_split.pop_front();
                        }
                    }
                }
            }
        }

        else if (input_split.front() == "go") {
            float search_time = DEFAULT_TIME;
            float search_time_max = DEFAULT_TIME_MAX;
            int maxply = MAX_PLY;
            bool force_time = false;
            input_split.pop_front();
            while (!input_split.empty()) {
                if (input_split.front() == "wtime") {
                    input_split.pop_front();
                    if (!board.side_to_move()) {
                        search_time = stof(input_split.front()) / 28000;
                        if (lichess_timing) { search_time -= 1.05f; }
                        search_time_max = stof(input_split.front()) / 5000;
                        if (lichess_timing) { search_time_max -= 1.05f; }
                    }
                    input_split.pop_front();
                }
                else if (input_split.front() == "btime") {
                    input_split.pop_front();
                    if (board.side_to_move()) {
                        search_time = stof(input_split.front()) / 28000;
                        if (AGI::lichess_timing) { search_time -= 1.05f; }
                        search_time_max = stof(input_split.front()) / 5000;
                        if (AGI::lichess_timing) { search_time_max -= 1.05f; }
                    }
                    input_split.pop_front();
                }
                else if (input_split.front() == "winc") {
                    input_split.pop_front();
                    if (!board.side_to_move()) {
                        search_time += stof(input_split.front()) / 1000;
                        search_time_max += stof(input_split.front()) / 1000;
                    }
                    input_split.pop_front();
                }
                else if (input_split.front() == "binc") {
                    input_split.pop_front();
                    if (board.side_to_move()) {
                        search_time += stof(input_split.front()) / 1000;
                        search_time_max += stof(input_split.front()) / 1000;
                    }
                    input_split.pop_front();
                }
                else if (input_split.front() == "movetime") {
                    input_split.pop_front();
                    search_time = stof(input_split.front()) / 1000;
                    force_time = true;
                    input_split.pop_front();
                }
                else if (input_split.front() == "infinite") {
                    input_split.pop_front();
                    search_time = -1;
                    search_time_max = -1;
                    force_time = true;
                    break;
                }
                else if (input_split.front() == "depth") {
                    input_split.pop_front();
                    maxply = stoi(input_split.front());
                    input_split.pop_front();
                }
                else { input_split.pop_front(); }
            }
            if (!force_time) {
                if (search_time < 0) { search_time = 0.05f; }
            }
            thread search = thread(SEARCH::search_start, &board, search_time, search_time_max, force_time, maxply, &ready_mutex);
            search.detach();
        }

        else if (input_split.front() == "stop") {
            Threads.stop = true;
        }

        else if (input_split.front() == "print") {
            cout << "mg: " << ((board.non_pawn_material() - 2000) >> 5) << "\n";
            cout << "end_eval function: " << AGI::eval_print(AGI::end_eval(board)) << "\n";
            cout << "eval function: " << AGI::eval_print(AGI::eval(board, AGI::QUIESCENCE_DEPTH)) << endl;
            cout << "repeat : " << board.repetition() << endl;
            cout << endl;

            // Move Generation
            MoveList<GenType::LEGAL> legal_moves = MoveList<GenType::LEGAL>(board);
            // Move Ordering and pre probing
            Move pbmove;
            int depth;
            for (auto m = legal_moves.begin(); m != legal_moves.end(); m++) {
                pbmove = MOVE_NULL;
                depth = 0;
                ((ExtMove*)m)->value = -Main_TT.probe(board.real_key_after(*m), 1, &pbmove);
                ((ExtMove*)m)->pbmove = pbmove;
            }
            legal_moves.sort();
            ExtMove* move;
            for (int i = 0; i < legal_moves.size(); i++) {
                move = legal_moves.list() + i;
                cout << AGI::move(*move) << " " << move->value << endl;
            }

        }

        else if (input_split.front() == "setoption") {
            input_split.pop_front();
            AGI::set_option(input_split);
        }

        else if (input_split.front() == "tree") {
            int a = 1;
            int b = 4;
            input_split.pop_front();
            if (!input_split.empty()) {
                a = stoi(input_split.front());
                input_split.pop_front();
                b = stoi(input_split.front());
                input_split.pop_front();
            }
            Stockfish::Move cmove;
            int k = 0;
            int eval = AGI::Main_TT.probe(board.key(), 0, &cmove, &k);
            cout << eval << " " << AGI::move(cmove) << " " << k << ":" << endl;
            SEARCH::show_tree(board, a, b);
        }

        else if (input_split.front() == "dcvb") {
            input_split.pop_front();
            cout << AGI::move(static_cast<Move>(stoi(input_split.front()))) << endl;
        }
    }

    return 88;
}

