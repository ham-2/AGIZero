#include <iostream>
#include <sstream>
#include <string>

#include "benchmark.h"
#include "board.h"
#include "threads.h"

using namespace std;
using namespace AGI;

void main() {
	
	string input, word;

	Board::init();
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

		}

		else if (word == "perft") {
			int depth;
			ss >> depth;
			perft(thread_manager.threads[0]->board, depth);
		}
	}

}