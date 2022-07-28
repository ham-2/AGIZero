#include "options.h"

namespace AGI {
	using namespace std;

	std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	bool lichess_timing = false;

	void print_option() {
		cout << "option name Threads type spin default " << AGI::SEARCH_THREADS_DEFAULT << " min 1 max " << AGI::SEARCH_THREADS_MAX << "\n"
			 << "option name Hash type spin default " << AGI::TABLE_MB_DEFAULT << " min " << AGI::TABLE_MB_DEFAULT << " max " << AGI::TABLE_MB_MAX << "\n"
			 << "option name PawnTable type spin default " << AGI::PAWN_TABLE_MB_DEFAULT << " min 0 max " << AGI::PAWN_TABLE_MB_MAX << "\n"
			 << "option name ATM type check default false\n"
			 << "option name LichessTiming type check default false\n"
			 << "option name Ponder type check default false\n"
			 << "option name Strength type spin default 100 min 0 max 100\n"
			 << "option name Contempt type spin default 0 min 0 max 100\n"
			 << endl;
	}

	void set_threads(int new_threads) {
		if (new_threads < SEARCH_THREADS_DEFAULT) { new_threads = SEARCH_THREADS_DEFAULT; }
		else if (new_threads > SEARCH_THREADS_MAX) { new_threads = SEARCH_THREADS_MAX; }

		// Increase Threads
		if (new_threads > SEARCH_THREADS) {
			while (new_threads != SEARCH_THREADS) {
				Stockfish::Position* thread_board = new Stockfish::Position;
				Threads.boards.push_back(thread_board);
				mutex* m = new mutex;
				Threads.vm.push_back(m);
				bool* k = new bool;
				*k = false;
				Threads.kill.push_back(k);
				thread v = thread(lazy_smp, thread_board, SEARCH_THREADS);
				v.detach();
				SEARCH_THREADS++;
			}
		}
		Threads.set_fen(DEFAULT_FEN);

		// Decrease Threads
		if (new_threads < SEARCH_THREADS) {
			while (new_threads != SEARCH_THREADS) {
				bool* k = Threads.kill.back();
				*k = true;
				t_wait.notify_all();
				while (*k) {
					this_thread::sleep_for(chrono::milliseconds(5));
				}
				Threads.kill.pop_back();
				delete k;
				delete Threads.boards.back();
				Threads.boards.pop_back();
				SEARCH_THREADS--;
			}
		}
	}

	void set_option(deque<string> input) {
		if (input.front() == "name") {
			input.pop_front();

			if (input.front() == "Threads") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					int new_threads = stoi(input.front());
					set_threads(new_threads);
				}
			}

			else if (input.front() == "Hash") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					int new_size = stoi(input.front());
					if (new_size < TABLE_MB_DEFAULT) { new_size = TABLE_MB_DEFAULT; }
					else {
						while ((new_size & (new_size - 1)) != 0) { new_size--; }
						if (new_size > TABLE_MB_MAX) { new_size = TABLE_MB_MAX; }
					}
					Main_TT.change_size((size_t)(new_size));
				}
			}

			else if (input.front() == "PawnTable") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					int new_size = stoi(input.front());
					if (new_size < 0) { new_size = 0; }
					else {
						while ((new_size & (new_size - 1)) != 0) { new_size--; }
						if (new_size > PAWN_TABLE_MB_MAX) { new_size = PAWN_TABLE_MB_MAX; }
					}
					Pawn_TT.change_size((size_t)(new_size));
				}
			}

			else if (input.front() == "ATM") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					if (input.front() == "true") {
						SEARCH::ATM = true;
					}
					else if (input.front() == "false") {
						SEARCH::ATM = false;
					}
				}
			}

			else if (input.front() == "LichessTiming") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					if (input.front() == "true") {
						lichess_timing = true;
					}
					else if (input.front() == "false") {
						lichess_timing = false;
					}
				}
			}

			else if (input.front() == "Ponder") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					if (input.front() == "true") {
						SEARCH::ponder = true;
					}
					else if (input.front() == "false") {
						SEARCH::ponder = false;
					}
				}
			}

			else if (input.front() == "Strength") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					int s = stoi(input.front());
					if (s > 99) { limit_strength = false; ls_p_max = 0; material_bias = 0; }
					else {
						if (s < 1) { s = 0; }
						limit_strength = true;
						ls_p_max = 300 - 3000 / (60 - s/2) + (100 - s) * (100 - s) / 12;
						material_bias = (100 - s);
					}
					psqt_init();
					psqt_bias(material_bias);
				}
			}

			else if (input.front() == "Contempt") {
				input.pop_front();
				if (input.front() == "value") {
					input.pop_front();
					int s = stoi(input.front());
					if (s > 100) { s = 100; }
					else if (s < 0) { s = 0; }
					contempt = -s;
				}
			}

		}
	}
}