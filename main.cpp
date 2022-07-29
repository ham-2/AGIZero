#include <iostream>
#include <sstream>
#include <string>

#include "benchmark.h"

using namespace std;
using namespace AGI;

void main() {
	
	string input, word;
	
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
			perft(depth);
		}
	}

}