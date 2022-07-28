#ifndef ALPHABETA_INCLUDED
#define ALPHABETA_INCLUDED

#include <deque>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>

#include "eval.h"
#include "types.h"
#include "position.h"
#include "movegen.h"
#include "constants.h"
#include "table.h"
#include "uci.h"

using namespace Stockfish;
using namespace std;

namespace AGI {

	struct SearchStackEntry {
		int dist_root;
		int old_score;
		Move nmove;

		SearchStackEntry() {
			dist_root = 0;
			old_score = 10000;
			nmove = MOVE_NULL;
		}
	};

	struct SearchStack {
		int thread_id;
		int pv_limit_depth;
		int same_move_count;
		int last_eval;
		int v_value;
		Move last_move;
		mutex m;
		vector<SearchStackEntry> ss;

		SearchStack() {
			thread_id = 0;
			same_move_count = 0;
			last_eval = EVAL_LOSS;
			last_move = MOVE_NULL;
			v_value = -60000;
			ss.clear();
		}

		void push() {
			SearchStackEntry* last_entry = &ss.back();
			ss.push_back(SearchStackEntry());
			ss.back().dist_root = last_entry->dist_root + 1;
		}

		void push(int eval, Move m) {
			SearchStackEntry* last_entry = &ss.back();
			ss.push_back(SearchStackEntry());
			ss.back().dist_root = last_entry->dist_root + 1;
			ss.back().old_score = eval;
			ss.back().nmove = m;
		}

		void pop() {
			ss.pop_back();
		}

		void init() {
			m.lock();
			ss.clear();
			ss.push_back(SearchStackEntry());
			same_move_count = 0;
			last_eval = EVAL_LOSS;
			v_value = -60000;
			m.unlock();
		}

		int currdist() {
			return ss.back().dist_root;
		}

		void push_v(int eval, Move mv) {
			m.lock();
			if (mv == last_move) { same_move_count++; }
			else { same_move_count = 0; last_move = mv; }
			v_value = v_value / 2;
			v_value = v_value + eval - last_eval;
			last_eval = eval;
			if (same_move_count < 3) { v_value *= (4 - same_move_count); }
			m.unlock();
		}

		int get_v() {
			m.lock();
			int temp = same_move_count ? v_value : v_value * 2;
			if (temp < 0) { temp = -temp; }
			m.unlock();
			return temp;
		}

		int pv_limit() {
			return pv_limit_depth - currdist();
		}

		void set_depth(int i) {
			pv_limit_depth = i;
		}

		int old_score() {
			return ss.back().old_score;
		}

		Move nmove() {
			return ss.back().nmove;
		}

		void set_back(int sc, Move m) {
			ss.back().old_score = sc;
			ss.back().nmove = m;
		}
	};

	int alpha_beta(Position& board, atomic<bool>* stop, SearchStack* ss, int ply, int* prev_red,
		int alpha = EVAL_LOSS, int beta = EVAL_WIN,
		int scb_us = EVAL_LOSS, int scb_them = EVAL_LOSS);

}

#endif