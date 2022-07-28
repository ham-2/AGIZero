#include "search.h"

using namespace std;

namespace SEARCH {

	bool ATM = false;
	bool ponder = false;
	atomic<bool> ponder_continue = false;

	SearchStack* main_ss = new SearchStack;

	string getpv(Position* board)
	{
		string pv;
		Move cmove = MOVE_NULL;
		Main_TT.probe(board->key(), 0, &cmove);
		if (cmove != MOVE_NULL) {
			board->do_move(cmove);
			pv = move(cmove).append(" ").append(getpv(board));
			board->undo_move(cmove);
		}
		else {
			pv = "";
		}
		return pv;
	}

	void clear1ply(Position& board) {
		MoveList<GenType::LEGAL> legal_moves = MoveList<GenType::LEGAL>(board);
		for (auto dmove = legal_moves.begin(); dmove != legal_moves.end(); dmove++) {
			board.do_move(*dmove);
			Main_TT.clear_entry(board.key());
			board.undo_move(*dmove);
		}
		Main_TT.clear_entry(board.key());
	}

	void show_tree(Position& board, int count, int max_v) {
		Move ccmove = MOVE_NULL;
		int var = max_v;
		Main_TT.probe(board.key(), 0, &ccmove);
		MoveList<GenType::LEGAL> legal_moves = MoveList<GenType::LEGAL>(board);
		vector<treetest> continuation;
		for (auto dmove = legal_moves.begin(); dmove != legal_moves.end(); dmove++) {
			board.do_move(*dmove);
			Move cmove;
			int k = 0;
			int eval = Main_TT.probe(board.key(), 0, &cmove, &k);
			continuation.push_back(treetest(*dmove, -eval, cmove, k));
			board.undo_move(*dmove);
		}
		std::sort(continuation.begin(), continuation.end(), [](const treetest lhs, const treetest rhs) { return lhs.eval > rhs.eval; });
		for (auto t = continuation.begin(); t != continuation.end(); t++) {
			if (var <= 0) { break; }
			var--;
			if (t->cmove != MOVE_NULL && t->cmove != MOVE_NONE) {
				board.do_move(t->cmove);
				cout << move(t->cmove) << " " << t->eval << " " << move(t->bmove) << " " << t->pad << "\n";
				if (count != 0) {
					int i = count;
					while (i > 0) { cout << "-----\n"; i--; }
					show_tree(board, count - 1, max_v);
				}
				board.undo_move(t->cmove);
			}
		}
		cout << endl;
	}

	bool atm(chrono::milliseconds curr_time, chrono::milliseconds limit_time, SearchStack* ss) {

		if (curr_time.count() * 100 / ATM_MIN_TIME < limit_time.count()) { return false; }

		// use print board
		MoveList<GenType::LEGAL> legal_moves = MoveList<GenType::LEGAL>(*Threads.boards[0]);
		Move nmove;

		if (legal_moves.size() < 2) { return true; }
		else {

			for (int i = 0; i < legal_moves.size(); i++) {
				ExtMove* m = legal_moves.list() + i;
				Threads.boards[0]->do_move(*m);
				m->value = Main_TT.probe(Threads.boards[0]->key(), 0, &nmove); // values are opposite color values
				if (m->value == -10000) { Threads.boards[0]->undo_move(*m); return false; } // Failed Search
				Threads.boards[0]->undo_move(*m);
			}
			
			sort(legal_moves.list(), legal_moves.list()+legal_moves.size());

			int curr_score = Main_TT.probe(Threads.boards[0]->key(), 0, &nmove);

			// Probing Error
			if (nmove != legal_moves.begin()->move) { return curr_time > limit_time; }

			// Compute
			int abs_diff = (legal_moves.begin() + 1)->value - legal_moves.begin()->value;
			int abs_var = main_ss->get_v() + ATM_STABILIZATION;

			// Time Adj
			abs_var = (int)(abs_var * (ATM_MG_BIAS * ATM_FLEXIBILITY + ATM_MG_BIAS * (200 - ATM_FLEXIBILITY) * log((double)limit_time.count() / (curr_time.count() + 1))) / 10000);

			// Cutoff
			if (abs_diff > abs_var) { return true; }
			else { return false; }

		}
	}

	void printer(float time, atomic<bool>* stop_flag, condition_variable* cv, float max_time, bool force_time, SearchStack* ss)
	{
		using namespace std::chrono;
		milliseconds limit_time{ static_cast<long int>(1000 * time) };
		milliseconds limit_time_max{ static_cast<long int>(1000 * max_time) };
		system_clock::time_point time_start = system_clock::now();
		milliseconds search_time = milliseconds(0);
		system_clock::time_point time_now;
		string pv;
		Move bmove = MOVE_NULL;
		int score = 0;
		mutex mu;

		while (!(*stop_flag)) {
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
			else { // Adaptive Search
				if (time == -1 || limit_time_max - search_time > milliseconds(PRINT_ATM_MS)) {
					cv->wait_for(m, milliseconds(PRINT_ATM_MS));
				}
				else {
					cv->wait_for(m, limit_time_max - search_time);
				}
			}

			// Limit Time
			time_now = system_clock::now();
			search_time = duration_cast<milliseconds>(time_now - time_start);

			if (force_time) { // Normal Search
				if (time != -1 && search_time >= limit_time) { 
					if (ponder) { ponder_continue = true; }
					Threads.stop = true;
					*stop_flag = true;
					Threads.sync();
				}
			}
			else { // Adaptive Search
				if (time != -1 && (search_time >= limit_time_max || atm(search_time, limit_time, ss))) {
					if (ponder) { ponder_continue = true; }
					Threads.stop = true;
					*stop_flag = true;
					Threads.sync();
				}
			}

			// Get candidate move
			score = Main_TT.probe(Threads.boards[0]->key(), 0, &bmove);
			pv = getpv(Threads.boards[0]);
			// Reset TTEntry if probe fails
			if (score == 10000) {
				if (!*stop_flag) { Main_TT.clear_entry(Threads.boards[0]->key()); continue; }
				else { cerr << "Probe Fail after Search" << endl; throw; }
			}

			// Print
			std::cout << "info time " << search_time.count() << " depth " << Threads.depth - 1
				<< " currmove " << move(bmove) << " score " << eval_print(score)
				<< " nodes " << node_count << " pv " << pv << endl;

			if ((*stop_flag)) { break; }
		}
		return;
	}

	void search_start(Position* board, float time, float max_time, bool force_time, int max_ply, std::mutex* ready_mutex)
	{
		Threads.stop = false;
		ready_mutex->lock();
		
		// Initialize Values
		int score = 0;
		int currdepth = 0;
		Move bmove = MOVE_NULL;
		atomic<bool>* complete = new atomic<bool>;
		*complete = false;
		condition_variable* print_cond = new condition_variable;
		force_time = !ATM || force_time;

		main_ss->init();
		node_count.exchange(0);
		Main_TT.increment();

		thread print_t = thread(printer, time, complete, print_cond, max_time, force_time, main_ss);

		Main_TT.clear_entry(board->key());
		Threads.depth.exchange(DEFAULT_PLY);
		main_ss->set_depth(Threads.depth);

		// Start parallel search
		t_wait.notify_all();
		
		while (Threads.depth <= max_ply) {

			Threads.vm[0]->lock();
			main_ss->set_back(10000, bmove);
			alpha_beta(*board, &(Threads.stop), main_ss, Threads.depth, &currdepth);
			Threads.vm[0]->unlock();

			//Forced Stop
			if (Threads.stop) { break; }

			// Get candidate move
			score = Main_TT.probe(board->key(), 0, &bmove);
			// Reset TTEntry if probe fails
			if (score == 10000) { Main_TT.clear_entry(board->key()); continue; }

			// Break if mate
			if (score > 29000 || score < -29000) {
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

			// ATM Op
			if (!force_time && score != 10000) { 
				main_ss->push_v(score, bmove);
			}

			// Depth
			if (currdepth >= Threads.depth){ 
				main_ss->set_depth(++Threads.depth);
				print_cond->notify_all(); // Print
			}
		}

		// terminate print thread
		*complete = true;
		print_cond->notify_all();
		print_t.join();
		delete complete;
		Threads.stop = true;
		Threads.sync();

		// Print Bestmove
		score = Main_TT.probe(board->key(), 0, &bmove);
		std::cout << "bestmove " << move(bmove) << endl;		

		// Ponder
		if (ponder_continue && !(score > 29000 || score < -29000)) {
			board->do_move(bmove);
			Threads.do_move(bmove);
			Threads.depth.exchange(DEFAULT_PLY);
			main_ss->init();
			Threads.stop = false;
			ponder_continue = false;

			// Start parallel search
			t_wait.notify_all();

			while (Threads.depth <= max_ply && !Threads.stop) {
				Threads.vm[0]->lock();
				alpha_beta(*board, &(Threads.stop), main_ss, Threads.depth, &currdepth);
				Threads.vm[0]->unlock();
				// Get candidate move
				score = Main_TT.probe(board->key(), 0, &bmove);

				// Reset TTEntry if probe fails
				if (score == 10000) { Main_TT.clear_entry(board->key()); continue; }

				// Break if mate
				if (score > 29000 || score < -29000) { break; }

				if (currdepth >= Threads.depth) { main_ss->set_depth(++Threads.depth); }
			}

			Threads.stop = true;
			Threads.sync();
		}

		ready_mutex->unlock();

		return;
	}

}