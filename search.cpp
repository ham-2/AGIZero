#include "search.h"

#include "alphabeta.h"
#include "eval.h"
#include "table.h"
#include "printer.h"

using namespace std;

namespace AGI {

	bool ATM = false;
	bool ponder = false;
	bool lichess_timing = false;
	atomic<bool> ponder_continue(false);

	void get_time(istringstream& ss, Color c, float& time, float& max_time, bool& force_time, int& max_ply) {
		string word;
		time = DEFAULT_TIME;
		max_time = DEFAULT_TIME_MAX;
		max_ply = MAX_PLY;
		force_time = false;
		while (ss >> word) {
			if (word == "wtime") {
				ss >> word;
				if (!c) {
					time = stof(word) / 28000;
					if (lichess_timing) { time -= 1.05f; }
					max_time = stof(word) / 5000;
					if (lichess_timing) { max_time -= 1.05f; }
				}
			}
			else if (word == "btime") {
				ss >> word;
				if (c) {
					time = stof(word) / 28000;
					if (lichess_timing) { time -= 1.05f; }
					max_time = stof(word) / 5000;
					if (lichess_timing) { max_time -= 1.05f; }
				}
			}
			else if (word == "winc") {
				ss >> word;
				if (!c) {
					time += stof(word) / 1000;
					max_time += stof(word) / 1000;
				}
			}
			else if (word == "binc") {
				ss >> word;
				if (c) {
					time += stof(word) / 1000;
					max_time += stof(word) / 1000;
				}
			}
			else if (word == "movetime") {
				ss >> word;
				time = stof(word) / 1000;
				force_time = true;
			}
			else if (word == "infinite") {
				ss >> word;
				time = -1;
				max_time = -1;
				force_time = true;
				break;
			}
			else if (word == "depth") {
				ss >> word;
				max_ply = stoi(word);
			}
		}
		if (!force_time) {
			if (time < 0) { time = 0.05f; }
		}
	}

	void clear1ply(Position& board) {
		MoveList legal_moves;
		legal_moves.generate(board);
		for (auto dmove = legal_moves.list; dmove != legal_moves.end; dmove++) {
			Undo u;
			board.do_move(*dmove, &u);
			Main_TT.clear_entry(board.get_key());
			board.undo_move(*dmove);
		}
		Main_TT.clear_entry(board.get_key());
	}

	void search_start(Thread* t, float time, float max_time, bool force_time, int max_ply)
	{
		Threads.stop = false;
		
		int score = 0;
		int currdepth = 0;
		int maxdepth = 0;
		int bmove_c = 0;
		int f_i = 0;
		Move bmove = NULL_MOVE;
		Move cmove;
		Move ccmove = NULL_MOVE;
		atomic<bool>* complete = new atomic<bool>;
		*complete = false;
		condition_variable* print_cond = new condition_variable;
		force_time = !ATM || force_time;
		Position* board = t->board;

		node_count.exchange(0);
		Main_TT.increment();

		thread print_t = thread(printer, time, complete, print_cond, max_time, force_time);

		Main_TT.clear_entry(board->get_key());
		Threads.depth.exchange(DEFAULT_PLY);
		currdepth = Threads.depth - 1;

		// Start parallel search
		Threads.t_wait.notify_all();
		
		Threads.threads[0]->m.lock();
		while (Threads.depth <= max_ply) {
			alpha_beta(*board, &(Threads.stop),
				Threads.depth, &currdepth,
				board->get_side(), t->step);

			//Forced Stop
			if (Threads.stop) { break; }

			// Get candidate move
			score = Main_TT.probe(board->get_key(), 0, &cmove);
			// Reset TTEntry if probe fails
			if (score == EVAL_FAIL) { Main_TT.clear_entry(board->get_key()); continue; }

			// Break if mate
			if (score > 29000 || score < -29000) {
				bmove = cmove;
				if (time == -1) {
					int ply_to_mate = 20000;
					if (score > 0) {
						ply_to_mate = EVAL_WIN - score;
					}
					else if (score < 0) {
						ply_to_mate = score + EVAL_WIN;
					}
					if (Threads.depth > 3 * ply_to_mate) { break; }
				}
				else { break; }
			}

			if (cmove != bmove && cmove == ccmove) { bmove_c++; }
			else { bmove_c = 0; }
			ccmove = cmove;
			if (currdepth > maxdepth) { maxdepth = currdepth; }

			// Choose best move & Depth
			if (bmove_c > BMOVE_CHANGE && currdepth > maxdepth / 2) { bmove = cmove; }
			if (currdepth >= Threads.depth) {
				bmove = cmove;
				Threads.depth++;
				f_i = 0;
				print_cond->notify_all(); // Print
			}
			else if (f_i++ > 32 * Threads.depth) {
				Threads.depth++;
				f_i = 0;
			}
		}

		// terminate print thread
		*complete = true;
		print_cond->notify_all();
		print_t.join();
		delete complete;
		Threads.stop = true;
		Threads.threads[0]->m.unlock();
		Threads.sync();

		// Print Bestmove
		std::cout << "bestmove " << move(bmove) << endl;		

		// Ponder
		if (ponder_continue && !(score > 29000 || score < -29000)) {
			Threads.do_move(bmove);
			Threads.depth.exchange(DEFAULT_PLY);
			Threads.stop = false;
			ponder_continue = false;

			// Start parallel search
			Threads.t_wait.notify_all();

			Threads.threads[0]->m.lock();
			while (Threads.depth <= max_ply && !Threads.stop) {
				alpha_beta(*board, &(Threads.stop),
					Threads.depth, &currdepth,
					board->get_side(), t->step);

				// Break if mate
				if (score > 29000 || score < -29000) { break; }

				if (currdepth >= Threads.depth || f_i > 32 * Threads.depth) {
					++Threads.depth;
					f_i = 0;
				}
				else { f_i++; }
			}

			Threads.stop = true;
			Threads.threads[0]->m.unlock();
		}

		return;
	}

}