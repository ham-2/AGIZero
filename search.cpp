#include "search.h"

#include "alphabeta.h"
#include "eval.h"
#include "table.h"
#include "printer.h"

using namespace std;

namespace AGI {

	bool ATM = false;
	bool lichess_timing = false;
	bool stop_if_mate = true;

	bool ponder = false;
	atomic<bool> ponder_continue(false);

	int multipv = 1;

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
		
		int multipv_max = multipv;
		Move bmove = NULL_MOVE;
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

		// Start parallel search
		Threads.t_wait.notify_all();
		Threads.threads[0]->m.lock();

		TTEntry probe = {};
		Main_TT.probe(board->get_key(), &probe);

		while (Threads.depth <= max_ply) {
			alpha_beta(*board, &(Threads.stop),
				Threads.depth, &probe,
				board->get_side(), t->step);

			// MultiPV search
			//if (multipv_max > 1) {
			//	// Move Generation
			//	MoveList legal_moves;
			//	legal_moves.generate(*board);
			//	if (legal_moves.length() < multipv_max) { multipv_max = legal_moves.length(); }
			//	vector<pair<Move, int>> pvmoves(legal_moves.length());

			//	// Update scores
			//	for (int i = 0; i < legal_moves.length(); i++) {
			//		Undo u;
			//		Move m = *(legal_moves.list + i);
			//		board->do_move(m, &u);
			//		TTEntry probe = {};
			//		if (Main_TT.probe(Threads.board->get_key(), &probe) == EVAL_FAIL) {
			//			pvmoves[i] = make_pair(m, -EVAL_FAIL);
			//		}
			//		else {
			//			pvmoves[i] = make_pair(m, -probe.eval);
			//		}
			//		board->undo_move(m);
			//	}

			//	// Find top moves
			//	sort(pvmoves.begin(), pvmoves.end(), compare);

			//	// Search Top Moves
			//	for (int i = 1; i < multipv_max; i++) {
			//		Undo u;
			//		board->do_move(pvmoves[i].first, &u);
			//		
			//		alpha_beta(*board, &(Threads.stop),
			//			Threads.depth - 1,
			//			board->get_side(), t->step);

			//		board->undo_move(pvmoves[i].first);
			//	}
			//}
			
			// Forced Stop
			if (Threads.stop) { break; }

			Main_TT.probe(board->get_key(), &probe);

			// Stop if mate
			if (stop_if_mate && (probe.eval > 29000 || probe.eval < -29000)) {
				break;
			}

			// Print and search again
			Threads.depth++;
			print_cond->notify_all();
		}

		// terminate print thread
		*complete = true;
		print_cond->notify_all();
		print_t.join();
		delete complete;
		Threads.stop = true;
		Threads.threads[0]->m.unlock();
		Threads.sync();

		//// limit strength - capture preference
		//if (limit_strength && score > -10000 && score < 10000) {
		//	MoveList legal_moves;
		//	legal_moves.generate(*board);
		//	int move_length = legal_moves.length();

		//	Move m;
		//	int comp_eval;
		//	int best_eval = EVAL_INIT;

		//	for (int i = 0; i < move_length; i++) {
		//		m = *(legal_moves.list + i);

		//		Undo u;
		//		board->do_move(m, &u);
		//		comp_eval = -Main_TT.probe(board->get_key(), 0);
		//		board->undo_move(m);

		//		if (comp_eval == EVAL_FAIL) { continue; }

		//		// capture preference
		//		if (board->material_capture(m)) {
		//			comp_eval += material_bias;
		//		}

		//		if (comp_eval > best_eval) {
		//			bmove = m;
		//			best_eval = comp_eval;
		//		}
		//	}
		//}

		// Print Bestmove
		Main_TT.probe(board->get_key(), &probe);
		Threads.acquire_cout();
		cout << "bestmove " << move(probe.nmove) << endl;		
		Threads.release_cout();

		// Ponder
		if (ponder_continue && !(probe.eval > 29000 || probe.eval < -29000)) {
			Threads.do_move(bmove);
			Threads.depth.exchange(DEFAULT_PLY);
			Threads.stop = false;
			ponder_continue = false;

			// Start parallel search
			Threads.t_wait.notify_all();

			Threads.threads[0]->m.lock();

			TTEntry probe_ponder = {};
			Main_TT.probe(board->get_key(), &probe_ponder);
			while (Threads.depth <= max_ply && !Threads.stop) {
				alpha_beta(*board, &(Threads.stop),
					Threads.depth, &probe_ponder,
					~(board->get_side()), t->step);

				Main_TT.probe(board->get_key(), &probe_ponder);
				++Threads.depth;
			}

			Threads.stop = true;
			Threads.threads[0]->m.unlock();
		}

		return;
	}

}