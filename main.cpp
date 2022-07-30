#include <iostream>
#include <sstream>
#include <string>

#include "benchmark.h"
#include "board.h"
#include "position.h"
#include "threads.h"

using namespace std;
using namespace AGI;

void main() {
	
	string input, word;

	Board::init();
	Position::init();
	thread_manager.init();

	while (true) {
		getline(cin, input);

		istringstream ss(input);
		word.clear();
		ss >> skipws >> word;

		if (word == "quit") { break; }

		else if (word == "uci") {

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

			thread_manager.set_all(fen);
			
			if (word == "moves") {
				
			}
		}

		else if (word == "perft") {
			int depth;
			ss >> depth;
			perft(thread_manager.threads[0]->board, depth);
		}

		else if (word == "showboard") {
			int threadidx;
			threadidx = ss >> threadidx ? threadidx : 0;
			thread_manager.show(threadidx);
		}
	}

}