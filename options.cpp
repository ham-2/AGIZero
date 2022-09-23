#include "options.h"

#include "eval.h"
#include "threads.h"

namespace AGI {
	using namespace std;

	std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	void print_option() {
		cout << "option name Threads type spin default " << 1 << " min 1 max " << SEARCH_THREADS_MAX << "\n"
			 << "option name Hash type spin default " << TABLE_MB_DEFAULT << " min " << TABLE_MB_DEFAULT << " max " << TABLE_MB_MAX << "\n"
			 << "option name PawnTable type spin default " << PAWN_TABLE_MB_DEFAULT << " min 0 max " << PAWN_TABLE_MB_MAX << "\n"
			 << "option name LichessTiming type check default false\n"
			 << "option name Ponder type check default false\n"
			 << "option name Contempt type spin default 0 min 0 max 100\n"
			 << endl;
	}

	void set_option(istringstream& ss) {
		string word;
		ss >> skipws >> word;
		if (word == "name") {
			ss >> word;
			if (word == "Threads") {
				ss >> word;
				if (word == "value") {
					int new_threads;
					ss >> new_threads;
					Threads.set_threads(new_threads);
				}
			}

			else if (word == "Hash") {
				ss >> word;
				if (word == "value") {
					int new_size;
					ss >> new_size;
					if (new_size < TABLE_MB_DEFAULT) { new_size = TABLE_MB_DEFAULT; }
					else {
						while ((new_size & (new_size - 1)) != 0) { new_size--; }
						if (new_size > TABLE_MB_MAX) { new_size = TABLE_MB_MAX; }
					}
					Main_TT.change_size((size_t)(new_size));
				}
			}

			else if (word == "PawnTable") {
				ss >> word;
				if (word == "value") {
					ss >> word;
					int new_size;
					ss >> new_size;
					if (new_size < 0) { new_size = 0; }
					else {
						while ((new_size & (new_size - 1)) != 0) { new_size--; }
						if (new_size > PAWN_TABLE_MB_MAX) { new_size = PAWN_TABLE_MB_MAX; }
					}
					Pawn_TT.change_size((size_t)(new_size));
				}
			}

			else if (word == "LichessTiming") {
				ss >> word;
				if (word == "value") {
					ss >> word;
					if (word == "true") {
						lichess_timing = true;
					}
					else if (word == "false") {
						lichess_timing = false;
					}
				}
			}

			else if (word == "Ponder") {
				ss >> word;
				if (word == "value") {
					ss >> word;
					if (word == "true") {
						ponder = true;
					}
					else if (word == "false") {
						ponder = false;
					}
				}
			}

			else if (word == "Contempt") {
				ss >> word;
				if (word == "value") {
					int s;
					ss >> s;
					if (s > 100) { s = 100; }
					else if (s < 0) { s = 0; }
					contempt = -s;
				}
			}

		}
	}
}