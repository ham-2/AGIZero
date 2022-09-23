#include "printer.h"

#include <sstream>

#include "board.h"
#include "constants.h"
#include "eval.h"
#include "search.h"
#include "table.h"
#include "threads.h"

using namespace std;

namespace AGI {

	void getpv(ostream& os, Position* board, int& depth) {
		string pv;
		Move cmove = NULL_MOVE;
		Main_TT.probe(board->get_key(), 0, &cmove);
		if (cmove != NULL_MOVE) {
			Undo u;
			board->do_move(cmove, &u);
			os << cmove << " ";
			depth++;
			getpv(os, board, depth);
			board->undo_move(cmove);
		}
	}

	void printer(float time, atomic<bool>* stop, condition_variable* cv, float max_time, bool force_time)
	{
		using namespace std::chrono;
		milliseconds limit_time{ static_cast<long int>(1000 * time) };
		milliseconds limit_time_max{ static_cast<long int>(1000 * max_time) };
		system_clock::time_point time_start = system_clock::now();
		milliseconds search_time = milliseconds(0);
		system_clock::time_point time_now;
		Move bmove = NULL_MOVE;
		int score = 0;
		int max_depth = 0;
		int currdepth;
		mutex mu;

		while (!(*stop)) {
			time_now = system_clock::now();
			search_time = duration_cast<milliseconds>(time_now - time_start);
			unique_lock<mutex> m(mu);
			if (force_time) { // Normal Search
				if (time == -1 || limit_time - search_time > milliseconds(PRINT_MIN_MS)) {
					cv->wait_for(m, milliseconds(PRINT_MIN_MS));
				}
				else {
					cv->wait_for(m, limit_time - search_time);
				}
			}

			// Limit Time
			time_now = system_clock::now();
			search_time = duration_cast<milliseconds>(time_now - time_start);

			if (force_time) { // Normal Search
				if (time != -1 && search_time >= limit_time) {
					if (ponder) { ponder_continue = true; }
					Threads.stop = true;
					*stop = true;
				}
			}

			if ((*stop)) { break; }

			// Get candidate move
			score = Main_TT.probe(Threads.board->get_key(), 0, &bmove);
			stringstream buf;
			currdepth = 0;
			getpv(buf, Threads.board, currdepth);
			// Reset TTEntry if probe fails
			if (score == EVAL_FAIL) {
				if (!*stop) { Main_TT.clear_entry(Threads.board->get_key()); continue; }
				else { cerr << "Probe Fail after Search" << endl; throw; }
			}
			if (currdepth > max_depth) { max_depth = currdepth; }

			// Print
			cout << "info time " << search_time.count() << " depth " << max_depth
				<< " currmove " << move(bmove) << " score " << eval_print(score)
				<< " nodes " << node_count << " pv " << buf.str() << endl;
		}

		// Get candidate move
		score = Main_TT.probe(Threads.board->get_key(), 0, &bmove);
		stringstream buf;
		getpv(buf, Threads.board, currdepth);

		// Print
		time_now = system_clock::now();
		search_time = duration_cast<milliseconds>(time_now - time_start);
		cout << "info time " << search_time.count() << " depth " << max_depth
			<< " currmove " << move(bmove) << " score " << eval_print(score)
			<< " nodes " << node_count << " pv " << buf.str() << endl;
		return;
	}

}