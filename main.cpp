#include <iostream>
#include <sstream>
#include <string>

#include "benchmark.h"
#include "board.h"
#include "material.h"
#include "options.h"
#include "position.h"
#include "search.h"
#include "threads.h"

using namespace std;
using namespace AGI;

int main() {
	
	string input, word;

	Board::init();
	material_init();
	Position::init();
	Threads.init();
	

	while (true) {
		getline(cin, input);

		istringstream ss(input);
		word.clear();
		ss >> skipws >> word;

		if (word == "quit") { break; }

		else if (word == "uci") {
			cout << "id name AGI\n"
				<< "id author Seungrae Kim" << endl;
			// Options
			print_option();
			cout << "uciok" << endl;
		}

		else if (word == "isready") {
			cout << "readyok" << endl;
		}
		
		else if (word == "position") {
			ss >> word;
			string fen;
			if (word == "fen") {
				while (ss >> word && word != "moves") {
					fen += word + " ";
				}
			}
			else if (word == "startpos") {
				fen = startpos_fen;
				ss >> word; // "moves"
			}

			Threads.acquire_lock();
			Threads.set_all(fen);
			if (word == "moves") {
				while (ss >> word) {
					Threads.do_move(word);
				}
			}
			Threads.release_lock();
		}

		else if (word == "go") {
			Color c = Threads.get_color();
			float time;
			float max_time;
			bool force_time;
			int max_ply;
			get_time(ss, c, time, max_time, force_time, max_ply);
			thread t = thread(search_start, Threads.threads[0], time, max_time, force_time, max_ply);
			t.detach();
		}

		else if (word == "stop") {
			Threads.stop = true;
		}

		else if (word == "setoption") {
			set_option(ss);
		}

		else if (word == "perft") {
			int depth;
			ss >> depth;
			perft(Threads.threads[0]->board, depth);
		}

		else if (word == "showboard") {
			int threadidx;
			threadidx = ss >> threadidx ? threadidx : 0;
			Threads.show(threadidx);
		}

		else if (word == "moves") {
			Threads.acquire_lock();
			while (ss >> word) {
				Threads.do_move(word);
			}
			Threads.release_lock();
		}

		else if (word == "generate") {
			Threads.gen();
		}

		else if (word == "test") {
			ss >> word;
			//Threads.see(word);
			Threads.test_eval();
		}

	}

	return 44;
}
